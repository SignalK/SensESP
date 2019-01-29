#include "networking.h"

#include <ESPAsyncWebServer.h>     //Local WebServer used to serve the configuration portal
#include <ESPAsyncWiFiManager.h>

#include "config.h"
#include "sensesp.h"
#include "system/led_blinker.h"

// networking is effectively a singleton, and the
// callback needs access to the object
Networking* networking;

void save_config_callback() {
  networking->save_configuration();
  ESP.restart();
}

Networking::Networking(String id, String schema) : Configurable{id, schema} {

}



void Networking::check_connection() {
  if (WiFi.status() != WL_CONNECTED) {
    // if connection is lost, simply restart
    ESP.restart();
  }
}

void Networking::setup(std::function<void(bool)> connection_cb) {
  char hostname[16];

  AsyncWebServer server(80);
  DNSServer dns;

  AsyncWiFiManager wifiManager(&server,&dns);

  //set config save notify callback
  wifiManager.setSaveConfigCallback(save_config_callback);

  wifiManager.setConfigPortalTimeout(WIFI_CONFIG_PORTAL_TIMEOUT);

  AsyncWiFiManagerParameter custom_hostname("hostname", "Set hostname", hostname, 16);
  wifiManager.addParameter(&custom_hostname);

  if (!wifiManager.autoConnect("Unconfigured Sensor")) {
    Serial.println(F("Failed to connect to wifi and config timed out."));
    ESP.restart();
  }

  Serial.println(F("Connected to Wifi."));
  connection_cb(true);

  app.onRepeat(1000, std::bind(Networking::check_connection, this));
}

