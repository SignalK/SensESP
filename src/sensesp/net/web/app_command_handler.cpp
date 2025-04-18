#include "app_command_handler.h"

#include <esp_http_server.h>
#include <memory>

#include "sensesp_app.h"

namespace sensesp {

void add_scan_wifi_networks_handlers(std::shared_ptr<HTTPServer>& server,
                                     std::shared_ptr<Networking>& networking) {
  auto scan_wifi_networks_handler = std::make_shared<HTTPRequestHandler>(
      1 << HTTP_POST, "/api/wifi/scan", [networking](httpd_req_t* req) {
        networking->start_wifi_scan();
        // Return status code 202 and "SCAN STARTED" message
        httpd_resp_set_status(req, "202 Accepted");
        httpd_resp_send(req, "SCAN STARTED", 0);
        return ESP_OK;
      });

  server->add_handler(scan_wifi_networks_handler);

  auto scan_results_handler = std::make_shared<HTTPRequestHandler>(
      1 << HTTP_GET, "/api/wifi/scan-results", [networking](httpd_req_t* req) {
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

void add_app_http_command_handlers(std::shared_ptr<HTTPServer>& server,
                                   std::shared_ptr<Networking>& networking) {
  add_scan_wifi_networks_handlers(server, networking);
}

}  // namespace sensesp
