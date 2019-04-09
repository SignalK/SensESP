#include "networking.h"

#include "config.h"
#include "sensesp.h"
#include "system/led_blinker.h"

bool should_save_config = false;

void save_config_callback() {
  should_save_config = true;
}

Networking::Networking(String id, String schema)
    : Configurable{id, schema} {
  hostname = new ObservableValue<String>(String("unknown"));
  load_configuration();
  server = new AsyncWebServer(80);
  dns = new DNSServer();
  wifi_manager = new AsyncWiFiManager(server, dns);
}

void Networking::check_connection() {
  if (WiFi.status() != WL_CONNECTED) {
    // if connection is lost, simply restart
    ESP.restart();
  }
}

void Networking::setup(std::function<void(bool)> connection_cb) {
  should_save_config = false;

  //set config save notify callback
  wifi_manager->setSaveConfigCallback(save_config_callback);

  wifi_manager->setConfigPortalTimeout(WIFI_CONFIG_PORTAL_TIMEOUT);

  #ifdef SERIAL_DEBUG_DISABLED
  wifi_manager->setDebugOutput(false);
  #endif
  AsyncWiFiManagerParameter custom_hostname(
    "hostname", "Set hostname", this->hostname->get().c_str(), 20);
  wifi_manager->addParameter(&custom_hostname);

  if (!wifi_manager->autoConnect("Unconfigured Sensor")) {
    debugE("Failed to connect to wifi and config timed out.");
    ESP.restart();
  }

  debugI("Connected to Wifi.");
  connection_cb(true);

  if (should_save_config) {
    String new_hostname = custom_hostname.getValue();
    debugI("Got new hostname: %s", new_hostname.c_str());
    this->hostname->set(new_hostname);
    save_configuration();
    debugW("Restarting in 500ms");
    app.onDelay(500, [](){ ESP.restart(); });
  }

  app.onRepeat(1000, std::bind(&Networking::check_connection, this));
}

ObservableValue<String>* Networking::get_hostname() {
  return this->hostname;
}

void Networking::set_hostname(String hostname) {
  debugD("Setting hostname");
  this->hostname->set(hostname);
}

JsonObject& Networking::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["hostname"] = this->hostname->get();
  return root;
}

bool Networking::set_configuration(const JsonObject& config) {
  if (!config.containsKey("hostname")) {
    return false;
  }
  this->hostname->set(config["hostname"].as<String>());
  return true;
}

void Networking::reset_settings() {
  wifi_manager->resetSettings();
}
