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

  void enable_wifi_manager(bool state) {
    wifi_manager_enabled_ = state;
  }

  void activate_wifi_manager();

  void set_wifi_manager_ap_ssid(String ssid) {
    wifi_manager_ap_ssid_ = ssid;
  }

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
  AsyncWiFiManager* wifi_manager = nullptr;

  bool wifi_manager_enabled_ = true;

  String ap_ssid = "";
  String ap_password = "";
  String wifi_manager_ap_ssid_ = "";

  /// hardcoded values provided as constructor parameters
  String preset_ssid = "";
  String preset_password = "";
  String preset_hostname = "";

  // original value of hardcoded hostname; used to detect changes
  // in the hardcoded value
  String default_hostname = "";

  const char* wifi_manager_password_;

};

}  // namespace sensesp

#endif
