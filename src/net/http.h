#ifndef _http_H_
#define _http_H_

#include <functional>

#include <ESPAsyncWebServer.h>

#include "system/startable.h"

/**
 * @brief Handles external interactions with the device via the web interface.
 */
class HTTPServer : public Startable {
 public:
  HTTPServer(std::function<void()> reset_device);
  ~HTTPServer() { delete server; }
  virtual void start() override { server->begin(); }
  void handle_not_found(AsyncWebServerRequest* request);
  void handle_config(AsyncWebServerRequest* request);
  void handle_device_reset(AsyncWebServerRequest* request);
  void handle_device_restart(AsyncWebServerRequest* request);
  void handle_info(AsyncWebServerRequest* request);

 private:
  AsyncWebServer* server;
  std::function<void()> reset_device;
  void handle_config_list(AsyncWebServerRequest* request);
};

#endif
