#ifndef SENSESP_NET_WEB_BASE_COMMAND_HANDLER_H_
#define SENSESP_NET_WEB_BASE_COMMAND_HANDLER_H_

#include <esp_http_server.h>

#include <vector>

#include "ArduinoJson.h"
#include "sensesp/net/http_server.h"
#include "sensesp/system/configurable.h"
#include "sensesp/ui/ui_output.h"
#include "sensesp_base_app.h"

namespace sensesp {

/**
 * @brief Handle HTTP requests to different action endpoints.
 *
 */
class HTTPResetHandler : public HTTPServerHandler {
 public:
  HTTPResetHandler() : HTTPServerHandler(){};
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

class HTTPRestartHandler : public HTTPServerHandler {
 public:
  HTTPRestartHandler() : HTTPServerHandler(){};
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

class HTTPInfoHandler : public HTTPServerHandler {
 public:
  HTTPInfoHandler() : HTTPServerHandler(){};
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

    std::size_t output_buffer_size =
        (200 * ui_outputs->size())     // status output entities
        + 512;
    DynamicJsonDocument json_doc(output_buffer_size);
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

class HTTPConfigListHandler : public HTTPServerHandler {
 public:
  HTTPConfigListHandler() : HTTPServerHandler(){};

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

  DynamicJsonDocument as_json() {
    DynamicJsonDocument doc(200);
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

class HTTPRoutesHandler : public HTTPServerHandler {
 public:
  HTTPRoutesHandler() : HTTPServerHandler() {
    this->get_routes();
  };

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
  };

 protected:
  std::vector<RouteDefinition> routes_;

  esp_err_t handle_get(httpd_req_t* req) {
    int buffer_size = 4000;
    DynamicJsonDocument json_doc(buffer_size);
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

class HTTPScanNetworksHandler : public HTTPServerHandler {
 public:
  HTTPScanNetworksHandler() : HTTPServerHandler(){};
  virtual void set_handler(HTTPServer* server) override {
    // handler for GET /device/restart
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
    int buffer_size = 4000;
    DynamicJsonDocument json_doc(buffer_size);
    JsonArray networks_json = json_doc.to<JsonArray>();

    int16_t num_networks = WiFi.scanNetworks();
    for (int i = 0; i < num_networks; i++) {
      JsonObject network_json = json_doc.createNestedObject();
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
