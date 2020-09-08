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

Networking::Networking(String config_path, String ssid,
                       String password, String hostname)
    : Configurable{config_path} {
  this->hostname = new ObservableValue<String>(hostname);

  preset_ssid = ssid;
  preset_password = password;
  preset_hostname = hostname;

  if (!ssid.isEmpty()) {
    debugI("Using hard-coded SSID %s and password",
           ssid.c_str());
    this->ap_ssid = ssid;
    this->ap_password = password;
  } else {
    load_configuration();
  }
  server = new AsyncWebServer(80);
  dns = new DNSServer();
  wifi_manager = new AsyncWiFiManager(server, dns);
}

void Networking::check_connection() {
  if (WiFi.status() != WL_CONNECTED) {
    // if connection is lost, simply restart
    debugD("Wifi disconnected: restarting...");
    ESP.restart();
  }
}

void Networking::setup(std::function<void(bool)> connection_cb) {
  if (ap_ssid != "" && ap_password != "") {
    setup_saved_ssid(connection_cb);
  }
  if (ap_ssid == "" && WiFi.status() != WL_CONNECTED) {
    setup_wifi_manager(connection_cb);
  }
  app.onRepeat(1000, std::bind(&Networking::check_connection, this));
}

void Networking::setup_saved_ssid(std::function<void(bool)> connection_cb) {
  WiFi.begin(ap_ssid.c_str(), ap_password.c_str());

  uint32_t timer_start = millis();

  debugI("Connecting to wifi %s.", ap_ssid.c_str());
  int printCounter = 0;
  while (WiFi.status() != WL_CONNECTED &&
         (millis() - timer_start) < 3 * 60 * 1000) {
    delay(500);
    if (printCounter % 4) {
      debugI("Wifi status=%d, time=%d ms", WiFi.status(), 500 * printCounter);
    }
    printCounter++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    debugI("Connected to wifi, SSID: %s (signal: %d)", WiFi.SSID().c_str(),
           WiFi.RSSI());
    connection_cb(true);
    WiFi.mode(WIFI_STA);
  }
}

void Networking::setup_wifi_manager(std::function<void(bool)> connection_cb) {
  should_save_config = false;

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

  if (!wifi_manager->autoConnect(pconfig_ssid)) {
    debugE("Failed to connect to wifi and config timed out. Restarting...");
    ESP.restart();
  }

  debugI("Connected to wifi,");
  debugI("IP address of Device: %s", WiFi.localIP().toString().c_str());
  connection_cb(true);

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

  return "\"" + key + "\":{\"title\":\"" + title + readonly_title + "\","
    + "\"type\":\"string\"" + readonly_property + "}";
}

String Networking::get_config_schema() {
  String schema;
  // If hostname is not set by SensESPAppBuilder::set_hostname() in main.cpp,
  // then preset_hostname will be "SensESP", and should not be read-only in the
  // Config UI. If preset_hostname is not "SensESP", then it was set in main.cpp, so
  // it should be read-only.
  bool hostname_preset = preset_hostname != "SensESP";
  bool wifi_preset = preset_ssid != "";
  return String(FPSTR(SCHEMA_PREFIX))
    + get_property_row("hostname", "ESP device hostname", hostname_preset) + ","
    + get_property_row("ap_ssid", "Wifi Access Point SSID", wifi_preset) + ","
    + get_property_row("ap_password", "Wifi Access Point Password", wifi_preset)
    + "}}";
}

JsonObject& Networking::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();

  root["hostname"] = this->hostname->get();
  root["ap_ssid"] = this->ap_ssid;
  root["ap_password"] = this->ap_password;

  return root;
}

bool Networking::set_configuration(const JsonObject& config) {
  if (!config.containsKey("hostname")) {
    return false;
  }

  if (preset_hostname == "SensESP") {
    this->hostname->set(config["hostname"].as<String>());
  }
  
  if (preset_ssid == "") {
    debugW("Ignoring saved SSID and password");
    this->ap_ssid = config["ap_ssid"].as<String>();
    this->ap_password = config["ap_password"].as<String>();
  }
  return true;
}

void Networking::reset_settings() {
  ap_ssid = preset_ssid;
  ap_password = preset_password;

  save_configuration();
  wifi_manager->resetSettings();
}
