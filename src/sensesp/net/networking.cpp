#include "networking.h"

#include "sensesp.h"
#include "sensesp/system/led_blinker.h"
#include "sensesp_app.h"

namespace sensesp {

// Wifi config portal timeout (seconds). The smaller the value, the faster
// the device will attempt to reconnect. If set too small, it might
// become impossible to actually configure the Wifi settings in the captive
// portal.
#ifndef WIFI_CONFIG_PORTAL_TIMEOUT
#define WIFI_CONFIG_PORTAL_TIMEOUT 180
#endif

// Network configuration logic:
// 1. Use hard-coded hostname and WiFi credentials by default
// 2. If the hostname or credentials have been changed in WiFiManager or
//    the web UI, use the updated values.
// 3. If the hard-coded hostname is changed, use that instead of the saved one.
//    (But keep using the saved WiFi credentials!)

Networking::Networking(String config_path, String client_ssid,
                       String client_password)
    : Configurable{config_path, "Basic WiFi Setup", 100}, Resettable(0) {
  // Get the WiFi state producer singleton and make it update this object output
  wifi_state_producer = WiFiStateProducer::get_singleton();
  wifi_state_producer->connect_to(new LambdaConsumer<WiFiState>(
      [this](WiFiState state) { this->emit(state); }));

  load_configuration();

  // Fill in the rest of the client settings array with empty configs
  int num_fill = kMaxNumClientConfigs - client_settings_.size();
  for (int i = 0; i < num_fill; i++) {
    client_settings_.push_back(ClientSSIDConfig());
  }

  if (client_ssid != "" && client_password != "" &&
      client_settings_.size() == 0) {
    ClientSSIDConfig preset_client_config = {client_ssid, client_password,
                                             true};
    client_settings_.push_back(preset_client_config);
    client_enabled_ = true;
  }

  debugD("Enabling Networking");

  // If both saved AP settings and saved client settings
  // are available, start in STA+AP mode.

  if (this->ap_settings_.enabled_ && this->client_enabled_ == true) {
    WiFi.mode(WIFI_AP_STA);
    start_access_point();
    start_client_autoconnect();
  }

  // If saved AP settings are available, use them.

  else if (this->ap_settings_.enabled_) {
    WiFi.mode(WIFI_AP);
    start_access_point();
  }

  // If saved client settings are available, use them.

  else if (this->client_enabled_) {
    WiFi.mode(WIFI_STA);
    start_client_autoconnect();
  } else {
    // Start WiFi with a bogus SSID to initialize the network stack but
    // don't connect to any network.
    WiFi.begin("0", "0");
  }

  if (this->ap_settings_.enabled_ &&
      this->ap_settings_.captive_portal_enabled_) {
    dns_server_ = new DNSServer();

    dns_server_->setErrorReplyCode(DNSReplyCode::NoError);
    dns_server_->start(53, "*", WiFi.softAPIP());

    ReactESP::app->onRepeat(1, [this]() { dns_server_->processNextRequest(); });
  }
}

/**
 * @brief Start an access point.
 *
 */
void Networking::start_access_point() {
  String hostname = SensESPBaseApp::get_hostname();
  WiFi.setHostname(hostname.c_str());

  debugI("Starting access point %s", ap_settings_.ssid_.c_str());

  bool result =
      WiFi.softAP(ap_settings_.ssid_.c_str(), ap_settings_.password_.c_str(),
                  ap_settings_.channel_, ap_settings_.hidden_);

  if (!result) {
    debugE("Failed to start access point.");
    return;
  }
}

/**
 * @brief Start WiFi using preset SSID and password.
 */
void Networking::start_client_autoconnect() {
  String hostname = SensESPBaseApp::get_hostname();
  WiFi.setHostname(hostname.c_str());

  // set up WiFi in regular STA (client) mode
  auto reconnect_cb = [this]() {
    static uint32_t attempt_num = 0;
    static uint32_t current_config_idx = 0;

    // First check if any of the client settings are defined
    if (client_settings_.size() == 0) {
      debugW("No client settings defined. Leaving WiFi client disconnected.");
      return;
    }

    uint32_t prev_config_idx = current_config_idx;

    ClientSSIDConfig config;

    while (true) {
      config = client_settings_[current_config_idx % client_settings_.size()];
      current_config_idx += 1;
      if (config.ssid_ != "" && config.password_ != "") {
        break;
      }
      if (current_config_idx == prev_config_idx + client_settings_.size()) {
        debugW(
            "No enabled client settings found. Leaving WiFi client "
            "disconnected.");
        return;
      }
    }

    if (client_enabled_ && WiFi.status() != WL_CONNECTED) {
      debugI("Connecting to wifi SSID %s (connection attempt #%d).",
             config.ssid_.c_str(), attempt_num);
      if (!config.use_dhcp_) {
        debugI("Using static IP address: %s", config.ip_.toString().c_str());
        WiFi.config(config.ip_, config.dns_server_, config.gateway_,
                    config.netmask_);
      }
      WiFi.begin(config.ssid_.c_str(), config.password_.c_str());
      attempt_num++;
    }
  };

  // Perform an initial connection without a delay.
  reconnect_cb();

  // Launch a separate onRepeat reaction to (re-)establish WiFi connection.
  // Connecting is attempted only every 20 s to allow the previous connection
  // attempt to complete even if the network is slow.
  ReactESP::app->onRepeat(20000, reconnect_cb);
}

/**
 * @brief Serialize the current configuration to a JSON document.
 *
 */
void Networking::get_configuration(JsonObject& root) {
  JsonObject apSettingsJson = root["apSettings"].to<JsonObject>();
  ap_settings_.as_json(apSettingsJson);

  JsonObject clientSettingsJson = root["clientSettings"].to<JsonObject>();
  clientSettingsJson["enabled"] = client_enabled_;
  JsonArray clientConfigsJson = clientSettingsJson["settings"].to<JsonArray>();
  int num_serialized = 0;
  for (auto& config : client_settings_) {
    if (num_serialized++ >= kMaxNumClientConfigs) {
      break;
    }
    JsonObject clientConfigJson = clientConfigsJson.add<JsonObject>();
    config.as_json(clientConfigJson);
  }
}

bool Networking::set_configuration(const JsonObject& config) {
  if (config.containsKey("hostname")) {
    // deal with the legacy Json format
    String hostname = config["hostname"].as<String>();
    SensESPBaseApp::get()->get_hostname_observable()->set(hostname);

    if (config.containsKey("ssid")) {
      String ssid = config["ssid"].as<String>();
      String password = config["password"].as<String>();

      if (config.containsKey("ap_mode")) {
        bool ap_mode;
        if (config["ap_mode"].as<String>() == "Access Point" ||
            config["ap_mode"].as<String>() == "Hotspot") {
          ap_settings_ = {true, ssid, password};
        } else {
          ClientSSIDConfig client_settings = {ssid, password};
          client_settings_.clear();
          client_settings_.push_back(client_settings);
          client_enabled_ = true;
        }
      }
    }
  } else {
    // Either an empty config or a new-style config
    if (config.containsKey("apSettings")) {
      ap_settings_ = AccessPointSettings::from_json(config["apSettings"]);
    } else {
      ap_settings_ = AccessPointSettings(true);
    }
    if (config.containsKey("clientSettings")) {
      const JsonObject& client_settings_json = config["clientSettings"];
      client_enabled_ = client_settings_json["enabled"] | false;
      client_settings_.clear();
      const JsonArray& client_settings_json_array =
          client_settings_json["settings"];
      for (const JsonObject& cfg_json : client_settings_json_array) {
        client_settings_.push_back(ClientSSIDConfig::from_json(cfg_json));
      }
      if (client_settings_.size() == 0) {
        client_enabled_ = false;
      }
    }
  }
  // Fill in the rest of the client settings array with empty configs
  while (client_settings_.size() < kMaxNumClientConfigs) {
    client_settings_.push_back(ClientSSIDConfig());
  }

  return true;
}

void Networking::reset() {
  debugI("Resetting WiFi SSID settings");

  clear_configuration();
  WiFi.disconnect(true);
  // On ESP32, disconnect does not erase previous credentials. Let's connect
  // to a bogus network instead
  WiFi.begin("0", "0");
}

WiFiStateProducer* WiFiStateProducer::instance_ = nullptr;

WiFiStateProducer* WiFiStateProducer::get_singleton() {
  if (instance_ == nullptr) {
    instance_ = new WiFiStateProducer();
  }
  return instance_;
}

void Networking::start_wifi_scan() {
  debugI("Starting WiFi network scan");
  WiFi.scanNetworks(true);
}

int16_t Networking::get_wifi_scan_results(
    std::vector<WiFiNetworkInfo>& ssid_list) {
  int num_networks = WiFi.scanComplete();
  if (num_networks == WIFI_SCAN_RUNNING) {
    return WIFI_SCAN_RUNNING;
  }
  if (num_networks == WIFI_SCAN_FAILED) {
    return WIFI_SCAN_FAILED;
  }
  ssid_list.clear();
  for (int i = 0; i < num_networks; i++) {
    WiFiNetworkInfo info(WiFi.SSID(i), WiFi.RSSI(i), WiFi.encryptionType(i),
                         WiFi.BSSID(i), WiFi.channel(i));
    ssid_list.push_back(info);
  }

  return num_networks;
}

}  // namespace sensesp
