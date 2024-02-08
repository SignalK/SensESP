#ifndef SENSESP_NET_WEB_BASE_COMMAND_HANDLER_H_
#define SENSESP_NET_WEB_BASE_COMMAND_HANDLER_H_

#include <esp_http_server.h>

#include <vector>

#include "ArduinoJson.h"
#include "sensesp/net/http_server.h"
#include "sensesp/net/web/autogen/web_ui_files.h"
#include "sensesp/net/web/static_file_handler.h"
#include "sensesp/system/configurable.h"
#include "sensesp/ui/ui_output.h"
#include "sensesp_base_app.h"

namespace sensesp {

/**
 * @brief Handle HTTP requests to different action endpoints.
 *
 */
class HTTPResetHandler : public HTTPRequestHandler {
 public:
  HTTPResetHandler() : HTTPRequestHandler(){};
  virtual void set_handler(HTTPServer* server) override {
    // handler for GET /device/reset
    const httpd_uri_t reset_handler = {
        .uri = "/api/device/reset",
        .method = HTTP_POST,
        .handler = [](httpd_req_t* req) {
          httpd_resp_send(req,
                          "Resetting device back to factory defaults. "
                          "You may have to reconfigure the WiFi settings.",
                          0);
          ReactESP::app->onDelay(500, []() { SensESPBaseApp::get()->reset(); });
          return ESP_OK;
        }};
    server->register_handler(&reset_handler);
  };
};

class HTTPRestartHandler : public HTTPRequestHandler {
 public:
  HTTPRestartHandler() : HTTPRequestHandler(){};
  virtual void set_handler(HTTPServer* server) override {
    // handler for GET /device/restart
    const httpd_uri_t restart_handler = {
        .uri = "/api/device/restart",
        .method = HTTP_POST,
        .handler = [](httpd_req_t* req) {
          httpd_resp_send(req, "Restarting device", 0);
          ReactESP::app->onDelay(500, []() { ESP.restart(); });
          return ESP_OK;
        }};
    server->register_handler(&restart_handler);
  };
};

class HTTPInfoHandler : public HTTPRequestHandler {
 public:
  HTTPInfoHandler() : HTTPRequestHandler(){};
  virtual void set_handler(HTTPServer* server) override {
    // handler for GET /api/info
    const httpd_uri_t info_handler = {
        .uri = "/api/info",
        .method = HTTP_GET,
        .handler =
            [](httpd_req_t* req) {
              return call_member_handler(req, &HTTPInfoHandler::handle_get);
            },
        .user_ctx = (void*)this,
    };
    server->register_handler(&info_handler);
  };

 protected:
  esp_err_t handle_get(httpd_req_t* req) {
    auto ui_outputs = UIOutputBase::get_ui_outputs();

    JsonDocument json_doc;
    JsonArray info_items = json_doc.to<JsonArray>();

    for (auto info_item = ui_outputs->begin(); info_item != ui_outputs->end();
         ++info_item) {
      info_items.add(info_item->second->as_json());
    }

    String response;
    serializeJson(json_doc, response);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, response.c_str());
    return ESP_OK;
  }
};

class HTTPConfigListHandler : public HTTPRequestHandler {
 public:
  HTTPConfigListHandler() : HTTPRequestHandler(){};

  void add_sorted_configurables(JsonArray& config) {
    // sort the configurables by their sort_order
    std::vector<std::pair<int, String>> pairs;
    for (auto it = configurables.begin(); it != configurables.end(); ++it) {
      pairs.push_back(std::make_pair(it->second->get_sort_order(), it->first));
    }

    std::sort(pairs.begin(), pairs.end());

    // add all configuration paths
    for (auto it = pairs.begin(); it != pairs.end(); ++it) {
      config.add(it->second);
    }
  }
};

class RouteDefinition {
 public:
  RouteDefinition(String name, String path, String component_name)
      : name_(name), path_(path), component_name_(component_name){};

  String get_name() { return name_; }
  String get_path() { return path_; }
  String get_component_name() { return component_name_; }

  JsonDocument as_json() {
    JsonDocument doc;
    doc["name"] = name_;
    doc["path"] = path_;
    doc["componentName"] = component_name_;

    return doc;
  }

 protected:
  String name_;
  String path_;
  String component_name_;
};

class HTTPRoutesHandler : public HTTPRequestHandler {
 public:
  HTTPRoutesHandler() : HTTPRequestHandler() { this->get_routes(); };

  virtual void set_handler(HTTPServer* server) override {
    // handler for GET /device/restart
    const httpd_uri_t routes_handler = {
        .uri = "/api/routes",
        .method = HTTP_GET,
        .handler =
            [](httpd_req_t* req) {
              return call_member_handler(req, &HTTPRoutesHandler::handle_get);
            },
        .user_ctx = (void*)this,
    };
    server->register_handler(&routes_handler);

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

    // Loop through routes and register to return the root page
    for (auto it = routes_.begin(); it != routes_.end(); ++it) {
      String path = it->get_path();
      const httpd_uri_t route_handler = {
          .uri = path.c_str(),
          .method = HTTP_GET,
          .handler =
              [](httpd_req_t* req) {
                return call_static_handler(
                    req, &HTTPStaticFileHandler::string_handler);
              },
          .user_ctx = (void*)root_page};
      server->register_handler(&route_handler);
    }
  };

 protected:
  std::vector<RouteDefinition> routes_;

  esp_err_t handle_get(httpd_req_t* req) {
    JsonDocument json_doc;
    JsonArray routes_json = json_doc.to<JsonArray>();

    int sz = routes_.size();

    for (auto it = routes_.begin(); it != routes_.end(); ++it) {
      routes_json.add(it->as_json());
    }

    String response;
    serializeJson(routes_json, response);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, response.c_str());
    return ESP_OK;
  }

  virtual void get_routes();
};

class HTTPScanNetworksHandler : public HTTPRequestHandler {
 public:
  HTTPScanNetworksHandler() : HTTPRequestHandler(){};
  virtual void set_handler(HTTPServer* server) override {
    const httpd_uri_t scan_networks_handler = {
        .uri = "/api/networks",
        .method = HTTP_GET,
        .handler =
            [](httpd_req_t* req) {
              return call_member_handler(req,
                                         &HTTPScanNetworksHandler::handle_get);
            },
        .user_ctx = (void*)this,
    };
    server->register_handler(&scan_networks_handler);
  };

 protected:
  esp_err_t handle_get(httpd_req_t* req) {
    JsonDocument json_doc;
    JsonArray networks_json = json_doc.to<JsonArray>();

    int16_t num_networks = WiFi.scanNetworks();
    for (int i = 0; i < num_networks; i++) {
      JsonObject network_json = json_doc.add<JsonObject>();
      network_json["ssid"] = WiFi.SSID(i);
      network_json["rssi"] = WiFi.RSSI(i);
      network_json["encryption"] = WiFi.encryptionType(i);
    }

    String response;
    serializeJson(json_doc, response);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, response.c_str());
    return ESP_OK;
  }
};

void add_base_app_http_command_handlers(HTTPServer* server);

}  // namespace sensesp

#endif  // SENSESP_NET_HTTP_COMMAND_HANDLER_H_
