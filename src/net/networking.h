#ifndef _networking_H_
#define _networking_H_

#include "Arduino.h"

// Local WebServer used to serve the configuration portal
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>

#include "system/configurable.h"
#include "system/observablevalue.h"

class Networking : public Configurable {
 public:
  Networking(String config_path);
  void setup(std::function<void(bool)> connection_cb);
  ObservableValue<String>* get_hostname();
  virtual JsonObject& get_configuration(JsonBuffer& buf) override final;
  virtual bool set_configuration(const JsonObject& config) override final;
  virtual String get_config_schema() override;

  //void set_hostname(String hostname);

  void reset_settings();

 protected:
  void check_connection();
  void setup_saved_ssid(std::function<void(bool)> connection_cb);
  void setup_wifi_manager(std::function<void(bool)> connection_cb);
 private:
  AsyncWebServer* server;
  // FIXME: DNSServer and AsyncWiFiManager could be instantiated in
  // respective methods to save some runtime memory
  DNSServer* dns;
  AsyncWiFiManager* wifi_manager;
  ObservableValue<String>* hostname;
  String ap_ssid = "";
  String ap_password = "";
};

#endif
