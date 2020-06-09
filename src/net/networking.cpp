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

void save_config_callback() {
  should_save_config = true;
}

Networking::Networking(String config_path)
    : Configurable{config_path} {
  hostname = new ObservableValue<String>(String("sensesp"));
  load_configuration();
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
  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi_manager(connection_cb);
  }
  app.onRepeat(1000, std::bind(&Networking::check_connection, this));
}

void Networking::setup_saved_ssid(std::function<void(bool)> connection_cb) {
  WiFi.begin(ap_ssid.c_str(), ap_password.c_str());

  uint32_t timer_start = millis();

  rdebugI("Connecting to wifi");
  while (WiFi.status() != WL_CONNECTED &&
         (millis() - timer_start) < 3 * 60 * 1000) {
    delay(500);
    rdebugI(".");
  }
  debugI("\n");

  if (WiFi.status() == WL_CONNECTED) {
    debugI("Connected to wifi, SSID: %s", WiFi.SSID().c_str());
    connection_cb(true);
  }
}

void Networking::setup_wifi_manager(std::function<void(bool)> connection_cb) {
  should_save_config = false;

  //set config save notify callback
  wifi_manager->setSaveConfigCallback(save_config_callback);

  wifi_manager->setConfigPortalTimeout(WIFI_CONFIG_PORTAL_TIMEOUT);

  #ifdef SERIAL_DEBUG_DISABLED
  wifi_manager->setDebugOutput(false);
  #endif
  AsyncWiFiManagerParameter custom_hostname(
    "hostname", "Set ESP Device custom hostname", this->hostname->get().c_str(), 20);
  wifi_manager->addParameter(&custom_hostname);

  // Create a unique SSID for configuring each SensESP Device
  String config_ssid = this->hostname->get();
  config_ssid = "Configure " + config_ssid;
  const char* pconfig_ssid = config_ssid.c_str();
    
  if (!wifi_manager->autoConnect(pconfig_ssid)) {
    debugE("Failed to connect to wifi and config timed out. Restarting...");
    ESP.restart();
  }

  debugI("Connected to wifi, SSID: %s", WiFi.SSID().c_str());
  debugI("IP address of Device: %s",  WiFi.localIP().toString().c_str());
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
    app.onDelay(500, [](){ ESP.restart(); });
  }
}

ObservableValue<String>* Networking::get_hostname() {
  return this->hostname;
}

// No longer used
//void Networking::set_hostname(String hostname) {
//  debugD("Setting hostname");
//  this->hostname->set(hostname);
//}

static const char SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "hostname": { "title": "ESP device hostname", "type": "string" },
        "ap_ssid": { "title": "Wifi Access Point SSID", "type": "string" },
        "ap_password": { "title": "Wifi Access Point Password", "type": "string" }
    }
  })";

String Networking::get_config_schema() {
  return FPSTR(SCHEMA);
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
  this->hostname->set(config["hostname"].as<String>());
  this->ap_ssid = config["ap_ssid"].as<String>();
  this->ap_password = config["ap_password"].as<String>();
  return true;
}

void Networking::reset_settings() {
  hostname->set("");
  ap_ssid = "";
  ap_password = "";
  save_configuration();
  wifi_manager->resetSettings();
}
