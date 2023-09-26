#ifndef SENSESP_NET_HTTP_COMMAND_HANDLER_H_
#define SENSESP_NET_HTTP_COMMAND_HANDLER_H_

#include "ArduinoJson.h"
#include "sensesp/net/http_server.h"
#include "sensesp/system/configurable.h"
#include "sensesp/ui/ui_output.h"
#include "sensesp_base_app.h"

#include <esp_http_server.h>

namespace sensesp {

/**
 * @brief Handle HTTP requests to different action endpoints.
 *
 */
class HTTPCommandHandler : public HTTPServerHandler {
 public:
  HTTPCommandHandler() : HTTPServerHandler(){};
  virtual void set_handler(HTTPServer* server) override {
    // handler for GET /device/reset
    const httpd_uri_t reset_handler = {
        .uri = "/device/reset",
        .method = HTTP_GET,
        .handler = [](httpd_req_t* req) {
          httpd_resp_send(req,
                          "Resetting device back to factory defaults. "
                          "You may have to reconfigure the WiFi settings.",
                          0);
          ReactESP::app->onDelay(500, []() { SensESPBaseApp::get()->reset(); });
          return ESP_OK;
        }};
    server->register_handler(&reset_handler);

    // handler for GET /device/restart
    const httpd_uri_t restart_handler = {
        .uri = "/device/restart",
        .method = HTTP_GET,
        .handler = [](httpd_req_t* req) {
          httpd_resp_send(req, "Restarting device", 0);
          ReactESP::app->onDelay(500, []() { ESP.restart(); });
          return ESP_OK;
        }};
    server->register_handler(&restart_handler);

    // handler for GET /info
    const httpd_uri_t info_handler = {
        .uri = "/info", .method = HTTP_GET, .handler = [](httpd_req_t* req) {
          return call_member_handler(req, &HTTPCommandHandler::handle_info_get);
        }};
    server->register_handler(&info_handler);
  };

 protected:
  esp_err_t handle_info_get(httpd_req_t* req) {
    auto ui_outputs = UIOutputBase::get_ui_outputs();

    std::size_t output_buffer_size =
        (200 * configurables.size() +  // configuration cards
         200 * ui_outputs->size())     // status output entities

        + 512;
    DynamicJsonDocument json_doc(output_buffer_size);

    auto properties = json_doc.createNestedObject("Properties");
    auto commands = json_doc.createNestedArray("Commands");
    auto pages = json_doc.createNestedArray("Pages");
    auto config = json_doc.createNestedArray("Config");

    for (auto property = ui_outputs->begin(); property != ui_outputs->end();
         ++property) {
      property->second->set_json(properties);
    }

    add_sorted_configurables(config);

    String response;
    serializeJson(json_doc, response);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, response.c_str());
    return ESP_OK;
  }

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

}  // namespace sensesp

#endif  // SENSESP_NET_HTTP_COMMAND_HANDLER_H_
