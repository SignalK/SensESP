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
  this->output = WiFiState::kWifiNoAP;

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

void Networking::setup_wifi_callbacks() {
  WiFi.onEvent([this](WiFiEvent_t event,
                      WiFiEventInfo_t info) { this->wifi_station_connected(); },
               WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(
      [this](WiFiEvent_t event, WiFiEventInfo_t info) {
        this->wifi_station_disconnected();
      },
      WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
}

/**
 * @brief Start WiFi using preset SSID and password.
 */
void Networking::setup_saved_ssid() {
  this->emit(WiFiState::kWifiDisconnected);
  setup_wifi_callbacks();

  String hostname = SensESPBaseApp::get_hostname();
  WiFi.setHostname(hostname.c_str());

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
}

/**
 * This method gets called when WiFi is connected to the AP and has
 * received an IP address.
 */
void Networking::wifi_station_connected() {
  debugI("Connected to wifi, SSID: %s (signal: %d)", WiFi.SSID().c_str(),
         WiFi.RSSI());
  debugI("IP address of Device: %s", WiFi.localIP().toString().c_str());
  debugI("Default route: %s", WiFi.gatewayIP().toString().c_str());
  debugI("DNS server: %s", WiFi.dnsIP().toString().c_str());
  this->emit(WiFiState::kWifiConnectedToAP);
}

/**
 * This method gets called when WiFi is disconnected from the AP.
 */
void Networking::wifi_station_disconnected() {
  debugI("Disconnected from wifi.");
  this->emit(WiFiState::kWifiDisconnected);
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

  setup_wifi_callbacks();

  // set config save notify callback
  wifi_manager->setBreakAfterConfig(true);

  wifi_manager->setConfigPortalTimeout(WIFI_CONFIG_PORTAL_TIMEOUT);

#ifdef SERIAL_DEBUG_DISABLED
  wifi_manager->setDebugOutput(false);
#endif
  AsyncWiFiManagerParameter custom_hostname(
      "hostname", "Set ESP32 device custom hostname", hostname.c_str(), 20);
  wifi_manager->addParameter(&custom_hostname);
  wifi_manager->setTryConnectDuringConfigPortal(false);

  // Create a unique SSID for configuring each SensESP Device
  String config_ssid;
  if (wifi_manager_ap_ssid_ != "") {
    config_ssid = wifi_manager_ap_ssid_;
  } else {
    config_ssid = "Configure " + hostname;
  }
  const char* pconfig_ssid = config_ssid.c_str();

  this->emit(WiFiState::kWifiManagerActivated);

  WiFi.setHostname(SensESPBaseApp::get_hostname().c_str());

  wifi_manager->startConfigPortal(pconfig_ssid, wifi_manager_password_);

  // WiFiManager attempts to connect to the new SSID, but that doesn't seem to
  // work reliably. Instead, we'll just attempt to connect manually.

  bool connected = false;
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
      "ssid": { "title": "WiFi SSID", "type": "string" },
      "password": { "title": "WiFi password", "type": "string", "format": "password" },
      "hostname": { "title": "Device hostname", "type": "string" }
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

}  // namespace sensesp
