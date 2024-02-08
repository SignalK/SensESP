#include "base_command_handler.h"

namespace sensesp {

void add_http_reset_handler(HTTPServer* server) {
  HTTPRequestHandler* reset_handler = new HTTPRequestHandler(
      1 << HTTP_POST, "/api/device/reset", [](httpd_req_t* req) {
        httpd_resp_send(req,
                        "Resetting device back to factory defaults. "
                        "You may have to reconfigure the WiFi settings.",
                        0);
        ReactESP::app->onDelay(500, []() { SensESPBaseApp::get()->reset(); });
        return ESP_OK;
      });
  server->add_handler(reset_handler);
}

void add_http_restart_handler(HTTPServer* server) {
  HTTPRequestHandler* restart_handler = new HTTPRequestHandler(
      1 << HTTP_POST, "/api/device/restart", [](httpd_req_t* req) {
        httpd_resp_send(req, "Restarting device", 0);
        ReactESP::app->onDelay(500, []() { ESP.restart(); });
        return ESP_OK;
      });
  server->add_handler(restart_handler);
}

void add_http_info_handler(HTTPServer* server) {
  HTTPRequestHandler* info_handler =
      new HTTPRequestHandler(1 << HTTP_GET, "/api/info", [](httpd_req_t* req) {
        auto ui_outputs = UIOutputBase::get_ui_outputs();

        JsonDocument json_doc;
        JsonArray info_items = json_doc.to<JsonArray>();

        for (auto info_item = ui_outputs->begin();
             info_item != ui_outputs->end(); ++info_item) {
          info_items.add(info_item->second->as_json());
        }

        String response;
        serializeJson(json_doc, response);
        httpd_resp_set_type(req, "application/json");
        httpd_resp_sendstr(req, response.c_str());
        return ESP_OK;
      });
  server->add_handler(info_handler);
}

void add_routes_handlers(HTTPServer* server) {
  std::vector<RouteDefinition> routes;

  routes.push_back(RouteDefinition("Status", "/status", "StatusPage"));
  routes.push_back(RouteDefinition("System", "/system", "SystemPage"));
  routes.push_back(RouteDefinition("WiFi", "/wifi", "WiFiConfigPage"));
  routes.push_back(RouteDefinition("Signal K", "/signalk", "SignalKPage"));
  routes.push_back(
      RouteDefinition("Configuration", "/configuration", "ConfigurationPage"));

  // Pre-render the response
  JsonDocument json_doc;
  JsonArray routes_json = json_doc.to<JsonArray>();

  int sz = routes.size();

  for (auto it = routes.begin(); it != routes.end(); ++it) {
    routes_json.add(it->as_json());
  }

  String response;

  serializeJson(routes_json, response);

  HTTPRequestHandler* routes_handler = new HTTPRequestHandler(
      1 << HTTP_GET, "/api/routes", [response](httpd_req_t* req) {
        httpd_resp_set_type(req, "application/json");
        httpd_resp_sendstr(req, response.c_str());
        return ESP_OK;
      });
  server->add_handler(routes_handler);

  // Find the root page

  StaticFileData* root_page = nullptr;
  for (int i = 0; i < sizeof(kWebUIFiles) / sizeof(StaticFileData); i++) {
    if (strcmp(kWebUIFiles[i].url, "/") == 0) {
      root_page = (StaticFileData*)&kWebUIFiles[i];
      break;
    }
  }
  if (root_page == nullptr) {
    debugE("Root page not found in kWebUIFiles");
    return;
  }

  // Add a handler for each route that returns the root page

  for (auto it = routes.begin(); it != routes.end(); ++it) {
    String path = it->get_path();
    HTTPRequestHandler* route_handler = new HTTPRequestHandler(
        1 << HTTP_GET, path.c_str(), [root_page](httpd_req_t* req) {
          httpd_resp_set_type(req, root_page->content_type);
          if (root_page->content_encoding != nullptr) {
            httpd_resp_set_hdr(req, kContentEncoding,
                               root_page->content_encoding);
          }
          httpd_resp_send(req, root_page->content, root_page->content_length);
          return ESP_OK;
        });
    server->add_handler(route_handler);
  }
}

void add_base_app_http_command_handlers(HTTPServer* server) {
  add_http_reset_handler(server);
  add_http_restart_handler(server);
  add_http_info_handler(server);
  add_routes_handlers(server);
}

}  // namespace sensesp
