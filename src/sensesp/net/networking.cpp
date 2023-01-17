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

Networking::Networking(String config_path, String ssid, String password,
                       String hostname, const char* wifi_manager_password)
    : Configurable{config_path, "Basic WiFi Setup", 100},
      wifi_manager_password_{wifi_manager_password},
      Startable(80),
      Resettable(0) {

  // Get the WiFi state producer singleton and make it update this object output
  wifi_state_producer = WiFiStateProducer::get_singleton();
  wifi_state_producer->connect_to(new LambdaConsumer<WiFiState>(
      [this](WiFiState state) { this->emit(state); }));

  preset_ssid = ssid;
  preset_password = password;
  preset_hostname = hostname;
  default_hostname = hostname;

  load_configuration();

  if (default_hostname != preset_hostname) {
    // if the preset hostname has changed, use it instead of the loaded one
    SensESPBaseApp::get()->get_hostname_observable()->set(preset_hostname);
    default_hostname = preset_hostname;
  }

  if (ap_ssid == "" && preset_ssid != "") {
    // there is no saved config and preset config is available
    ap_ssid = preset_ssid;
  }

  if (ap_password == "" && preset_password != "") {
    // there is no saved config and preset config is available
    ap_password = preset_password;
  }

  server = new AsyncWebServer(80);
  dns = new DNSServer();
}

void Networking::start() {
  debugD("Enabling Networking object");

  // If we have preset or saved WiFi config, always use it. Otherwise,
  // start WiFiManager. WiFiManager always starts the configuration portal
  // instead of trying to connect.

  if (ap_ssid != "" && ap_password != "") {
    debugI("Using SSID %s", ap_ssid.c_str());
    setup_saved_ssid();
  } else if (ap_ssid == "" && WiFi.status() != WL_CONNECTED &&
             wifi_manager_enabled_) {
    debugI("Starting WiFiManager");
    setup_wifi_manager();
  }
  // otherwise, fall through and WiFi will remain disconnected
}

void Networking::activate_wifi_manager() {
  debugD("Activating WiFiManager");
  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi_manager();
  }
}

/**
 * @brief Start WiFi using preset SSID and password.
 */
void Networking::setup_saved_ssid() {
  String hostname = SensESPBaseApp::get_hostname();
  WiFi.setHostname(hostname.c_str());

  if (ap_mode_ == false) {
    // set up WiFi in regular STA (client) mode
    auto reconnect_cb = [this]() {
      if (WiFi.status() != WL_CONNECTED) {
        debugI("Connecting to wifi SSID %s.", ap_ssid.c_str());
        WiFi.begin(ap_ssid.c_str(), ap_password.c_str());
      }
    };

    // Perform an initial connection without a delay.
    reconnect_cb();

    // Launch a separate onRepeat reaction to (re-)establish WiFi connection.
    // Connecting is attempted only every 20 s to allow the previous connection
    // attempt to complete even if the network is slow.
    ReactESP::app->onRepeat(20000, reconnect_cb);
  } else {
    // Set up WiFi in AP mode. In this case, we don't need a reconnect loop.
    debugI("Setting up a WiFi access point...");
    WiFi.softAP(ap_ssid.c_str(), ap_password.c_str());
  }
}

/**
 * @brief Start WiFi using WiFi Manager.
 *
 * If the setup process has been completed before, this method will start
 * the WiFi connection using the saved SSID and password. Otherwise, it will
 * start the WiFi Manager.
 */
void Networking::setup_wifi_manager() {
  wifi_manager = new AsyncWiFiManager(server, dns);

  String hostname = SensESPBaseApp::get_hostname();

  // set config save notify callback
  wifi_manager->setBreakAfterConfig(true);

  wifi_manager->setConfigPortalTimeout(WIFI_CONFIG_PORTAL_TIMEOUT);

#ifdef SERIAL_DEBUG_DISABLED
  wifi_manager->setDebugOutput(false);
#endif
  AsyncWiFiManagerParameter custom_hostname("hostname", "Device hostname",
                                            hostname.c_str(), 64);
  wifi_manager->addParameter(&custom_hostname);

  AsyncWiFiManagerParameter custom_ap_ssid(
      "ap_ssid", "Custom Access Point SSID", ap_ssid.c_str(), 33);
  wifi_manager->addParameter(&custom_ap_ssid);

  AsyncWiFiManagerParameter custom_ap_password(
      "ap_password", "Custom Access Point Password", ap_password.c_str(), 64);
  wifi_manager->addParameter(&custom_ap_password);

  wifi_manager->setTryConnectDuringConfigPortal(false);

  // Create a unique SSID for configuring each SensESP Device
  String config_ssid;
  if (wifi_manager_ap_ssid_ != "") {
    config_ssid = wifi_manager_ap_ssid_;
  } else {
    config_ssid = "Configure " + hostname;
  }
  const char* pconfig_ssid = config_ssid.c_str();

  // this is the only WiFi state we actively still emit
  this->emit(WiFiState::kWifiManagerActivated);

  WiFi.setHostname(SensESPBaseApp::get_hostname().c_str());

  wifi_manager->startConfigPortal(pconfig_ssid, wifi_manager_password_);

  String configured_custom_ap_ssid = custom_ap_ssid.getValue();
  String configured_custom_ap_password = custom_ap_password.getValue();

  bool connected = false;

  if (configured_custom_ap_ssid != "" && configured_custom_ap_password != "") {
    // AP mode is desired
    ap_ssid = configured_custom_ap_ssid;
    ap_password = configured_custom_ap_password;
    ap_mode_ = true;

    // always assume we can launch a soft AP
    connected = true;
  } else {
    // WiFiManager attempts to connect to the new SSID, but that doesn't seem to
    // work reliably. Instead, we'll just attempt to connect manually.

    this->ap_ssid = wifi_manager->getConfiguredSTASSID();
    this->ap_password = wifi_manager->getConfiguredSTAPassword();

    // attempt to connect with the new SSID and password
    if (this->ap_ssid != "" && this->ap_password != "") {
      debugD("Attempting to connect to acquired SSID %s and password",
             this->ap_ssid.c_str());
      WiFi.begin(this->ap_ssid.c_str(), this->ap_password.c_str());
      for (int i = 0; i < 20; i++) {
        if (WiFi.status() == WL_CONNECTED) {
          connected = true;
          break;
        }
        delay(1000);
      }
    }
  }

  // Only save the new configuration if we were able to connect to the new SSID.

  if (connected) {
    String new_hostname = custom_hostname.getValue();
    debugI("Got new custom hostname: %s", new_hostname.c_str());
    SensESPBaseApp::get()->get_hostname_observable()->set(new_hostname);
    debugI("Got new SSID and password: %s", ap_ssid.c_str());
    save_configuration();
  }
  debugW("Restarting...");
  ESP.restart();
}

String Networking::get_config_schema() {
  static const char kSchema[] = R"###({
    "type": "object",
    "properties": {
      "hostname": { "title": "Device hostname", "type": "string" },
      "ssid": { "title": "WiFi SSID", "type": "string" },
      "password": { "title": "WiFi password", "type": "string", "format": "password" },
      "ap_mode": { "type": "string", "format": "radio", "title": "WiFi mode", "enum": ["Client", "Access Point"] }
    }
  })###";

  return String(kSchema);
}

// FIXME: hostname should be saved in SensESPApp

void Networking::get_configuration(JsonObject& root) {
  String hostname = SensESPBaseApp::get_hostname();
  root["hostname"] = hostname;
  root["default_hostname"] = default_hostname;
  root["ssid"] = ap_ssid;
  root["password"] = ap_password;
  root["ap_mode"] = ap_mode_ ? "Access Point" : "Client";
}

bool Networking::set_configuration(const JsonObject& config) {
  if (!config.containsKey("hostname")) {
    return false;
  }

  SensESPBaseApp::get()->get_hostname_observable()->set(
      config["hostname"].as<String>());

  if (config.containsKey("default_hostname")) {
    default_hostname = config["default_hostname"].as<String>();
  }
  ap_ssid = config["ssid"].as<String>();
  ap_password = config["password"].as<String>();

  if (config.containsKey("ap_mode")) {
    if (config["ap_mode"].as<String>() == "Access Point" ||
        config["ap_mode"].as<String>() == "Hotspot") {
      ap_mode_ = true;
    } else {
      ap_mode_ = false;
    }
  }

  return true;
}

void Networking::reset() {
  debugI("Resetting WiFi SSID settings");

  ap_ssid = preset_ssid;
  ap_password = preset_password;

  save_configuration();
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

}  // namespace sensesp
