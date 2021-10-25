#ifndef _networking_H_
#define _networking_H_

#include "Arduino.h"

#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif

// Local WebServer used to serve the configuration portal
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>

#include "system/configurable.h"
#include "system/startable.h"
#include "system/observablevalue.h"
#include "system/valueproducer.h"

enum class WifiState {
  kWifiNoAP = 0,
  kWifiDisconnected,
  kWifiConnectedToAP,
  kWifiManagerActivated
};

/**
 * @brief Manages the ESP's connection to the Wifi network.
 */
class Networking : public Configurable, public Startable, public ValueProducer<WifiState> {
 public:
  Networking(String config_path, String ssid, String password, String hostname);
  virtual void start() override;
  virtual void get_configuration(JsonObject& doc) override final;
  virtual bool set_configuration(const JsonObject& config) override final;
  virtual String get_config_schema() override;

  void reset_settings();

 protected:
  void setup_saved_ssid();
  void setup_wifi_callbacks();
  void setup_wifi_manager();

  // callbacks

  void wifi_station_connected();
  void wifi_station_disconnected();

 private:
  AsyncWebServer* server;
  // FIXME: DNSServer and AsyncWiFiManager could be instantiated in
  // respective methods to save some runtime memory
  DNSServer* dns;
  AsyncWiFiManager* wifi_manager;

#ifdef ESP8266
  // event handlers must be saved to keep the handlers activated
  WiFiEventHandler got_ip_event_handler_;
  WiFiEventHandler wifi_disconnected_event_handler_;
#endif

  String ap_ssid = "";
  String ap_password = "";
  String preset_ssid = "";
  String preset_password = "";
  String preset_hostname = "";
};

#endif
