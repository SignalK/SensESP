#include "command_handler.h"

namespace sensesp {

void HTTPRoutesHandler::get_routes() {
    routes_.push_back(RouteDefinition("Status", "/status", "StatusPage"));
    routes_.push_back(RouteDefinition("System", "/system", "SystemPage"));
    routes_.push_back(RouteDefinition("WiFi", "/wifi", "WiFiConfigPage"));
    routes_.push_back(RouteDefinition("Signal K", "/signalk", "SignalKPage"));
    routes_.push_back(RouteDefinition("Configuration", "/configuration",
                                     "ConfigurationPage"));
  }

void add_http_command_handlers(HTTPServer* server) {
  HTTPResetHandler* reset_handler = new HTTPResetHandler();
  HTTPRestartHandler* restart_handler = new HTTPRestartHandler();
  HTTPInfoHandler* info_handler = new HTTPInfoHandler();
  HTTPRoutesHandler* routes_handler = new HTTPRoutesHandler();
}

}  // namespace sensesp
