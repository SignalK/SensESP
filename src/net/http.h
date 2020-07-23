#ifndef _http_H_
#define _http_H_

#include <functional>

#include <ESPAsyncWebServer.h>
#include "sensesp_app_options.h"

class HTTPServer {
 public:
  HTTPServer(std::function<void()> reset_device, SensESPAppOptions*options);
  ~HTTPServer() { delete server; }
  void enable() { server->begin(); }
  void handle_not_found(AsyncWebServerRequest* request);
  void handle_config(AsyncWebServerRequest* request);
  void handle_device_reset(AsyncWebServerRequest* request);
  void handle_device_restart(AsyncWebServerRequest* request);
  void handle_info(AsyncWebServerRequest* request);
 private:
  AsyncWebServer* server;
  std::function<void()> reset_device;
  void handle_config_list(AsyncWebServerRequest* request);
  SensESPAppOptions*options;
};


#endif
