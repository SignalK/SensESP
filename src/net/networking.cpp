#include "networking.h"

#include <ESPAsyncWebServer.h>     //Local WebServer used to serve the configuration portal
#include <ESPAsyncWiFiManager.h>

#include "config.h"
#include "sensesp.h"
#include "system/led_blinker.h"


bool should_save_config = false;

void save_config_callback() {
  Serial.println(F("Should save config"));
  should_save_config = true;
}

void check_connection() {
  if (WiFi.status() != WL_CONNECTED) {
    // if connection is lost, simply restart
    ESP.restart();
  }
}

void setup_networking(LedBlinker led_blinker) {
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
  led_blinker.set_wifi_connected();

  // Save config if needed
  if (should_save_config) {
    strcpy(hostname, custom_hostname.getValue());
    //saveConfig();
    // For some reason, connection isn't properly established after exiting the
    // captive portal. Just reset to be safe.
    ESP.restart();
  }

  app.onRepeat(1000, check_connection);
}

