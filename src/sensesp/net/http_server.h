#ifndef _http_H_
#define _http_H_

#include <ESPAsyncWebServer.h>

#include <functional>

#include "sensesp/ui/ui_button.h"
#include "sensesp/system/startable.h"

namespace sensesp {

/**
 * @brief Handles external interactions with the device via the web interface.
 */
class HTTPServer : public Startable {
 public:
  HTTPServer();
  ~HTTPServer() { delete server; }
  virtual void start() override;
  void handle_not_found(AsyncWebServerRequest* request);
  void handle_config(AsyncWebServerRequest* request);
  void handle_device_reset(AsyncWebServerRequest* request);
  void handle_device_restart(AsyncWebServerRequest* request);
  void handle_info(AsyncWebServerRequest* request);
  void handle_static_reponse(AsyncWebServerRequest* request,
                             const uint8_t* content, uint32_t size);

  void handle_command(AsyncWebServerRequest* request);

 private:
  AsyncWebServer* server;
  void handle_config_list(AsyncWebServerRequest* request);
};

}  // namespace sensesp

#endif
