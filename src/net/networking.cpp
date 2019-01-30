#include "networking.h"

// Local WebServer used to serve the configuration portal
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>

#include "config.h"
#include "sensesp.h"
#include "system/led_blinker.h"

bool should_save_config = false;

void save_config_callback() {
  should_save_config = true;
}

Networking::Networking(String id, String schema)
    : Configurable{id, schema} {
  hostname = new ObservableValue<String>(String(""));
  load_configuration();
}

void Networking::check_connection() {
  if (WiFi.status() != WL_CONNECTED) {
    // if connection is lost, simply restart
    ESP.restart();
  }
}

void Networking::setup(std::function<void(bool)> connection_cb) {
  AsyncWebServer server(80);
  DNSServer dns;
  should_save_config = false;

  AsyncWiFiManager wifiManager(&server,&dns);

  //set config save notify callback
  wifiManager.setSaveConfigCallback(save_config_callback);

  wifiManager.setConfigPortalTimeout(WIFI_CONFIG_PORTAL_TIMEOUT);

  AsyncWiFiManagerParameter custom_hostname(
    "hostname", "Set hostname", this->hostname->get().c_str(), 20);
  wifiManager.addParameter(&custom_hostname);

  if (should_save_config) {
    this->hostname->set(custom_hostname.getValue());
    save_configuration();
    ESP.restart();
  }

  if (!wifiManager.autoConnect("Unconfigured Sensor")) {
    Serial.println(F("Failed to connect to wifi and config timed out."));
    ESP.restart();
  }

  Serial.println(F("Connected to Wifi."));
  connection_cb(true);

  app.onRepeat(1000, std::bind(&Networking::check_connection, this));
}

ObservableValue<String>* Networking::get_hostname() {
  return this->hostname;
}

void Networking::set_hostname(String hostname) {
  this->hostname->set(hostname);
}

JsonObject& Networking::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["hostname"] = this->hostname->get();
  return root;
}

void Networking::set_configuration(const JsonObject& config) {
  this->hostname->set(config["hostname"].as<String>());
  this->save_configuration();
  // network configuration changes require a device restart
  app.onDelay(50, [](){ ESP.restart(); });
}
