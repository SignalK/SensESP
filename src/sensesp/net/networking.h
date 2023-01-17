#ifndef _networking_H_
#define _networking_H_

#include "Arduino.h"

// Local WebServer used to serve the configuration portal
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>

#include "sensesp/net/wifi_state.h"
#include "sensesp/system/configurable.h"
#include "sensesp/system/observablevalue.h"
#include "sensesp/system/resettable.h"
#include "sensesp/system/startable.h"
#include "sensesp/system/valueproducer.h"

namespace sensesp {

/**
 * @brief Provide information about the current WiFi state.
 *
 * WiFiStateProducer reads the current network state using
 * Arduino Core callbacks. It is a replacement for the Networking class
 * ValueProducer output and effectively decouples the Networkig class
 * from the rest of the system. This allows for replacing the Networking
 * class with a different implementation.
 */
class WiFiStateProducer : public ValueProducer<WiFiState>, public Startable {
 public:
  /**
   * Singletons should not be cloneable
   */
  WiFiStateProducer(WiFiStateProducer& other) = delete;

  /**
   * Singletons should not be assignable
   */
  void operator=(const WiFiStateProducer&) = delete;

  /**
   * @brief Get the singleton instance of the WiFiStateProducer
   */
  static WiFiStateProducer* get_singleton();

  virtual void start() override {
    setup_wifi_callbacks();
    // Emit the current state immediately
    this->emit(this->output);
  }

 protected:
  WiFiStateProducer() : Startable(81) { this->output = WiFiState::kWifiNoAP; }

  void setup_wifi_callbacks() {
    WiFi.onEvent(
        [this](WiFiEvent_t event, WiFiEventInfo_t info) {
          this->wifi_station_connected();
        },
        WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent([this](WiFiEvent_t event,
                        WiFiEventInfo_t info) { this->wifi_ap_enabled(); },
                 WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_START);
    WiFi.onEvent([this](WiFiEvent_t event,
                        WiFiEventInfo_t info) { this->wifi_disconnected(); },
                 WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    WiFi.onEvent([this](WiFiEvent_t event,
                        WiFiEventInfo_t info) { this->wifi_disconnected(); },
                 WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_STOP);
  }

  void wifi_station_connected() {
    debugI("Connected to wifi, SSID: %s (signal: %d)", WiFi.SSID().c_str(),
           WiFi.RSSI());
    debugI("IP address of Device: %s", WiFi.localIP().toString().c_str());
    debugI("Default route: %s", WiFi.gatewayIP().toString().c_str());
    debugI("DNS server: %s", WiFi.dnsIP().toString().c_str());
    this->emit(WiFiState::kWifiConnectedToAP);
  }

  void wifi_ap_enabled() {
    debugI("WiFi Access Point enabled, SSID: %s", WiFi.softAPSSID().c_str());
    debugI("IP address of Device: %s", WiFi.softAPIP().toString().c_str());

    // Setting the AP mode happens immediately,
    // so this callback is likely called already before all startables have been
    // initiated. Delay the WiFi state update until the start of the event loop.
    ReactESP::app->onDelay(
        0, [this]() { this->emit(WiFiState::kWifiAPModeActivated); });
  }

  void wifi_disconnected() {
    debugI("Disconnected from wifi.");
    this->emit(WiFiState::kWifiDisconnected);
  }

  static WiFiStateProducer* instance_;
};

/**
 * @brief Manages the ESP's connection to the Wifi network.
 */
class Networking : public Configurable,
                   public Startable,
                   public Resettable,
                   public ValueProducer<WiFiState> {
 public:
  Networking(String config_path, String ssid, String password, String hostname,
             const char* wifi_manager_password);
  virtual void start() override;
  virtual void reset() override;

  virtual void get_configuration(JsonObject& doc) override final;
  virtual bool set_configuration(const JsonObject& config) override final;
  virtual String get_config_schema() override;

  void enable_wifi_manager(bool state) { wifi_manager_enabled_ = state; }

  void activate_wifi_manager();

  void set_wifi_manager_ap_ssid(String ssid) { wifi_manager_ap_ssid_ = ssid; }

  void set_ap_mode(bool state) { ap_mode_ = state; }

 protected:
  void setup_saved_ssid();
  void setup_wifi_manager();

  // callbacks

  void wifi_station_connected();
  void wifi_ap_enabled();
  void wifi_disconnected();

 private:
  AsyncWebServer* server;
  // FIXME: DNSServer and AsyncWiFiManager could be instantiated in
  // respective methods to save some runtime memory
  DNSServer* dns;
  AsyncWiFiManager* wifi_manager = nullptr;

  bool wifi_manager_enabled_ = true;

  // If true, the device will set up its own WiFi access point

  bool ap_mode_ = false;

  // values provided by WiFiManager or saved from previous configuration

  String ap_ssid = "";
  String ap_password = "";

  // hardcoded values provided as constructor parameters

  String wifi_manager_ap_ssid_ = "";
  String preset_ssid = "";
  String preset_password = "";
  String preset_hostname = "";

  // original value of hardcoded hostname; used to detect changes
  // in the hardcoded value
  String default_hostname = "";

  WiFiStateProducer* wifi_state_producer;

  const char* wifi_manager_password_;
};

}  // namespace sensesp

#endif
