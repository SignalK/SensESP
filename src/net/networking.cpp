#include "networking.h"

#include "sensesp.h"
#include "system/led_blinker.h"

// Wifi config portal timeout (seconds). The smaller the value, the faster
// the device will attempt to reconnect. If set too small, it might
// become impossible to actually configure the Wifi settings in the captive
// portal.
#ifndef WIFI_CONFIG_PORTAL_TIMEOUT
#define WIFI_CONFIG_PORTAL_TIMEOUT 180
#endif

bool should_save_config = false;

void save_config_callback() { should_save_config = true; }

Networking::Networking(String config_path, String ssid, String password,
                       String hostname)
    : Configurable{config_path} {
  this->hostname = new ObservableValue<String>(hostname);

  this->output = WifiState::kWifiNoAP;

  preset_ssid = ssid;
  preset_password = password;
  preset_hostname = hostname;

  if (!ssid.isEmpty()) {
    debugI("Using hard-coded SSID %s and password", ssid.c_str());
    this->ap_ssid = ssid;
    this->ap_password = password;
  } else {
    load_configuration();
  }
  server = new AsyncWebServer(80);
  dns = new DNSServer();
  wifi_manager = new AsyncWiFiManager(server, dns);
}

void Networking::setup() {
  if (ap_ssid != "" && ap_password != "") {
    setup_saved_ssid();
  }
  if (ap_ssid == "" && WiFi.status() != WL_CONNECTED) {
    setup_wifi_manager();
  }
}

void Networking::setup_wifi_callbacks() {
#if defined(ESP8266)
  got_ip_event_handler_ =
      WiFi.onStationModeGotIP([this](const WiFiEventStationModeGotIP& event) {
        this->wifi_station_connected();
      });
  wifi_disconnected_event_handler_ = WiFi.onStationModeDisconnected(
      [this](const WiFiEventStationModeDisconnected& event) {
        this->wifi_station_disconnected();
      });
#elif defined(ESP32)
  WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
    this->wifi_station_connected();
  }, WiFiEvent_t::SYSTEM_EVENT_STA_GOT_IP);
  WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
    this->wifi_station_disconnected();
  }, WiFiEvent_t::SYSTEM_EVENT_STA_DISCONNECTED);  
#endif
}

void Networking::setup_saved_ssid() {
  this->emit(WifiState::kWifiDisconnected);
  setup_wifi_callbacks();
  WiFi.begin(ap_ssid.c_str(), ap_password.c_str());

  debugI("Connecting to wifi %s.", ap_ssid.c_str());
}

void Networking::wifi_station_connected() {
    debugI("Connected to wifi, SSID: %s (signal: %d)", WiFi.SSID().c_str(),
           WiFi.RSSI());
    debugI("IP address of Device: %s", WiFi.localIP().toString().c_str());
    this->emit(WifiState::kWifiConnectedToAP);
}

void Networking::wifi_station_disconnected() {
  debugI("Disconnected from wifi.");
  this->emit(WifiState::kWifiDisconnected);
}

void Networking::setup_wifi_manager() {
  should_save_config = false;

  setup_wifi_callbacks();

  // set config save notify callback
  wifi_manager->setSaveConfigCallback(save_config_callback);

  wifi_manager->setConfigPortalTimeout(WIFI_CONFIG_PORTAL_TIMEOUT);

#ifdef SERIAL_DEBUG_DISABLED
  wifi_manager->setDebugOutput(false);
#endif
  AsyncWiFiManagerParameter custom_hostname("hostname",
                                            "Set ESP Device custom hostname",
                                            this->hostname->get().c_str(), 20);
  wifi_manager->addParameter(&custom_hostname);

  // Create a unique SSID for configuring each SensESP Device
  String config_ssid = this->hostname->get();
  config_ssid = "Configure " + config_ssid;
  const char* pconfig_ssid = config_ssid.c_str();

  this->emit(WifiState::kWifiManagerActivated);

  if (!wifi_manager->autoConnect(pconfig_ssid)) {
    debugE("Failed to connect to wifi and config timed out. Restarting...");

    this->emit(WifiState::kWifiDisconnected);

    ESP.restart();
  }

  debugI("Connected to wifi,");
  debugI("IP address of Device: %s", WiFi.localIP().toString().c_str());
  this->emit(WifiState::kWifiConnectedToAP);

  if (should_save_config) {
    String new_hostname = custom_hostname.getValue();
    debugI("Got new custom hostname: %s", new_hostname.c_str());
    this->hostname->set(new_hostname);
    this->ap_ssid = WiFi.SSID();
    debugI("Got new SSID and password: %s", ap_ssid.c_str());
    this->ap_password = WiFi.psk();
    save_configuration();
    debugW("Restarting in 500ms");
    app.onDelay(500, []() { ESP.restart(); });
  }
}

ObservableValue<String>* Networking::get_hostname() { return this->hostname; }

static const char SCHEMA_PREFIX[] PROGMEM = R"({
"type": "object",
"properties": {
)";

String get_property_row(String key, String title, bool readonly) {
  String readonly_title = "";
  String readonly_property = "";

  if (readonly) {
    readonly_title = " (readonly)";
    readonly_property = ",\"readOnly\":true";
  }

  return "\"" + key + "\":{\"title\":\"" + title + readonly_title + "\"," +
         "\"type\":\"string\"" + readonly_property + "}";
}

String Networking::get_config_schema() {
  String schema;
  // If hostname is not set by SensESPAppBuilder::set_hostname() in main.cpp,
  // then preset_hostname will be "SensESP", and should not be read-only in the
  // Config UI. If preset_hostname is not "SensESP", then it was set in
  // main.cpp, so it should be read-only.
  bool hostname_preset = preset_hostname != "SensESP";
  return String(FPSTR(SCHEMA_PREFIX)) +
         get_property_row("hostname", "ESP device hostname", hostname_preset) +
         "}}";
}

void Networking::get_configuration(JsonObject& root) {
  root["hostname"] = this->hostname->get();
}

bool Networking::set_configuration(const JsonObject& config) {
  if (!config.containsKey("hostname")) {
    return false;
  }

  if (preset_hostname == "SensESP") {
    this->hostname->set(config["hostname"].as<String>());
  }

  return true;
}

void Networking::reset_settings() {
  ap_ssid = preset_ssid;
  ap_password = preset_password;

  save_configuration();
  WiFi.disconnect(true);
  // On ESP32, disconnect does not erase previous credentials. Let's connect
  // to a bogus network instead
  WiFi.begin("0", "0");
}
