#ifndef _http_H_
#define _http_H_

#include <ESPAsyncWebServer.h>

#include <functional>

#include "sensesp/system/startable.h"
#include "http_command.h"

namespace sensesp {

/**
 * @brief Handles external interactions with the device via the web interface.
 */
class HTTPServer : public Startable {
 public:
  HTTPServer();
  ~HTTPServer() { delete server; }
  virtual void start() override { server->begin(); }
  void handle_not_found(AsyncWebServerRequest* request);
  void handle_config(AsyncWebServerRequest* request);
  void handle_device_reset(AsyncWebServerRequest* request);
  void handle_device_restart(AsyncWebServerRequest* request);
  void handle_info(AsyncWebServerRequest* request);
  void handle_static_reponse(AsyncWebServerRequest* request, const uint8_t*content, uint32_t size);
  void handle_command(AsyncWebServerRequest* request);
  HTTPCommand* add_command(String name, String title, bool mustConfirm = true);
 private:
  AsyncWebServer* server;
  void handle_config_list(AsyncWebServerRequest* request);
};

}  // namespace sensesp

#endif
