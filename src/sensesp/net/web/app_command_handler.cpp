#include "app_command_handler.h"

#include <esp_http_server.h>

namespace sensesp {

void add_scan_wifi_networks_handlers(HTTPServer* server) {
  HTTPRequestHandler* scan_wifi_networks_handler =
      new HTTPRequestHandler(1 << HTTP_POST, "/api/wifi/scan", [](httpd_req_t* req) {
        auto networking = SensESPApp::get()->get_networking();
        networking->start_wifi_scan();
        // Return status code 202 and "SCAN STARTED" message
        httpd_resp_set_status(req, "202 Accepted");
        httpd_resp_send(req, "SCAN STARTED", 0);
        return ESP_OK;
      });

  server->add_handler(scan_wifi_networks_handler);

  HTTPRequestHandler* scan_results_handler =
      new HTTPRequestHandler(1 << HTTP_GET, "/api/wifi/scan-results",
                             [](httpd_req_t* req) {
        auto networking = SensESPApp::get()->get_networking();
        std::vector<WiFiNetworkInfo> ssid_list;
        int16_t result = networking->get_wifi_scan_results(ssid_list);
        if (result == WIFI_SCAN_RUNNING) {
          // Return status code 202 and "SCAN RUNNING" message
          httpd_resp_set_status(req, "202 Accepted");
          httpd_resp_send(req, "SCAN RUNNING", 0);
          return ESP_OK;
        } else if (result == WIFI_SCAN_FAILED) {
          // Return status code 400 and "SCAN FAILED" message
          httpd_resp_set_status(req, "400 Bad Request");
          httpd_resp_send(req, "SCAN FAILED", 0);
          return ESP_OK;
        }
        // Construct a JSON array of the scan results
        JsonDocument doc;
        JsonArray networks = doc["networks"].to<JsonArray>();
        for (auto& ssid_info : ssid_list) {
          JsonObject network = networks.add<JsonObject>();
          ssid_info.as_json(network);
        }
        String json_str;
        serializeJson(doc, json_str);
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, json_str.c_str(), json_str.length());
        return ESP_OK;
      });

  server->add_handler(scan_results_handler);
}

void add_app_http_command_handlers(HTTPServer* server) {
  add_scan_wifi_networks_handlers(server);
}

}  // namespace sensesp
