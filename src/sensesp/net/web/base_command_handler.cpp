#include "base_command_handler.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <new>

#include "sensesp/net/web/autogen/frontend_files.h"
#include "sensesp/system/log_buffer.h"
#include "sensesp/ui/status_page_item.h"
#include "sensesp_app.h"

namespace sensesp {

// An Origin or Host that does not fit the buffer is rejected rather than
// treated as absent: a legitimate same-origin request to this device is
// always short, so an over-long header can only be a forgery attempt and
// must fail closed.
bool check_origin(httpd_req_t* req) {
  if (httpd_req_get_hdr_value_len(req, "Origin") == 0) {
    return true;
  }

  char origin[128] = {0};
  char host[128] = {0};
  if (httpd_req_get_hdr_value_str(req, "Origin", origin, sizeof(origin)) !=
          ESP_OK ||
      httpd_req_get_hdr_value_str(req, "Host", host, sizeof(host)) != ESP_OK) {
    httpd_resp_send_err(req, HTTPD_403_FORBIDDEN,
                        "Cross-origin request rejected");
    return false;
  }

  String origin_str(origin);
  int scheme_end = origin_str.indexOf("://");
  String origin_authority =
      scheme_end < 0 ? origin_str : origin_str.substring(scheme_end + 3);

  if (origin_authority == host) {
    return true;
  }

  httpd_resp_send_err(req, HTTPD_403_FORBIDDEN,
                      "Cross-origin request rejected");
  return false;
}

void add_http_reset_handler(std::shared_ptr<HTTPServer>& server) {
  auto reset_handler = std::make_shared<HTTPRequestHandler>(
      1 << HTTP_POST, "/api/device/reset", [](httpd_req_t* req) {
        if (!check_origin(req)) {
          return ESP_FAIL;
        }
        httpd_resp_send(req,
                        "Resetting device back to factory defaults. "
                        "You may have to reconfigure the WiFi settings.",
                        0);
        event_loop()->onDelay(500, []() { SensESPBaseApp::get()->reset(); });
        return ESP_OK;
      });
  server->add_handler(reset_handler);
}

void add_http_restart_handler(std::shared_ptr<HTTPServer>& server) {
  auto restart_handler = std::make_shared<HTTPRequestHandler>(
      1 << HTTP_POST, "/api/device/restart", [](httpd_req_t* req) {
        if (!check_origin(req)) {
          return ESP_FAIL;
        }
        httpd_resp_send(req, "Restarting device", 0);
        event_loop()->onDelay(500, []() { ESP.restart(); });
        return ESP_OK;
      });
  server->add_handler(restart_handler);
}

void add_http_info_handler(std::shared_ptr<HTTPServer>& server) {
  auto info_handler = std::make_shared<HTTPRequestHandler>(
      1 << HTTP_GET, "/api/info", [](httpd_req_t* req) {
        auto status_page_items = StatusPageItemBase::get_status_page_items();

        JsonDocument json_doc;
        JsonArray info_items = json_doc.to<JsonArray>();

        for (auto info_item = status_page_items->begin();
             info_item != status_page_items->end(); ++info_item) {
          info_items.add(info_item->second->as_json());
        }

        // Per-task minimum-ever free stack, grouped under "Task stack free
        // (bytes)", for spotting over- or under-provisioned task stacks.
        // Computed live per request; needs the FreeRTOS trace facility, which
        // the Arduino/ESP-IDF default config enables.
#if defined(CONFIG_FREERTOS_USE_TRACE_FACILITY) && \
    CONFIG_FREERTOS_USE_TRACE_FACILITY
        // Over-allocate a few slots so a task created between the count and the
        // snapshot can't undersize the array: uxTaskGetSystemState returns 0 on
        // an undersized buffer, which would drop the whole section that request.
        UBaseType_t task_slots = uxTaskGetNumberOfTasks() + 4;
        std::unique_ptr<TaskStatus_t[]> tasks(
            new (std::nothrow) TaskStatus_t[task_slots]);
        if (tasks) {
          UBaseType_t n = uxTaskGetSystemState(tasks.get(), task_slots, nullptr);
          for (UBaseType_t i = 0; i < n; i++) {
            JsonDocument item;
            item["name"] = tasks[i].pcTaskName;
            item["value"] = static_cast<uint32_t>(
                tasks[i].usStackHighWaterMark * sizeof(StackType_t));
            item["group"] = "Task stack free (bytes)";
            item["order"] = kUIOutputDefaultOrder;
            info_items.add(item);
          }
        }
#endif

        String response;
        serializeJson(json_doc, response);
        httpd_resp_set_type(req, "application/json");
        httpd_resp_sendstr(req, response.c_str());
        return ESP_OK;
      });
  server->add_handler(info_handler);
}

void add_http_log_handler(std::shared_ptr<HTTPServer>& server) {
  auto log_handler = std::make_shared<HTTPRequestHandler>(
      1 << HTTP_GET, "/api/log", [](httpd_req_t* req) {
        LogBuffer* log_buffer = LogBuffer::instance();
        httpd_resp_set_type(req, "application/json; charset=utf-8");
        if (log_buffer == nullptr) {
          httpd_resp_sendstr(
              req, "{\"session\":0,\"next\":0,\"gap\":false,\"lines\":[]}");
          return ESP_OK;
        }

        // Access control: none of its own, like /api/info. The only gate is the
        // dispatcher's HTTP auth, which is off by default — so /api/log mirrors
        // the serial log to anyone who can reach the web server unless web auth
        // is enabled. check_origin() is anti-CSRF for destructive POSTs and
        // gives no read protection, so it is not used here.
        uint32_t since = 0;
        bool has_since = false;
        size_t query_len = httpd_req_get_url_query_len(req) + 1;
        if (query_len > 1 && query_len <= 64) {
          char query[64];
          if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
            char value[16];
            if (httpd_query_key_value(query, "since", value, sizeof(value)) ==
                ESP_OK) {
              // Accept only a clean, in-range unsigned integer; ignore garbage
              // so a stale or crafted cursor cannot corrupt the client's view.
              char* end = nullptr;
              errno = 0;
              unsigned long parsed = strtoul(value, &end, 10);
              if (end != value && *end == '\0' && errno == 0 &&
                  parsed <= UINT32_MAX) {
                since = static_cast<uint32_t>(parsed);
                has_since = true;
              }
            }
          }
        }

        LogSnapshot snapshot = log_buffer->snapshot_since(since, has_since);

        JsonDocument json_doc;
        json_doc["session"] = snapshot.session_id;
        json_doc["next"] = snapshot.next;
        json_doc["gap"] = snapshot.gap;
        JsonArray lines = json_doc["lines"].to<JsonArray>();
        for (const auto& line : snapshot.lines) {
          lines.add(line.c_str());
        }

        String response;
        serializeJson(json_doc, response);
        httpd_resp_sendstr(req, response.c_str());
        return ESP_OK;
      });
  server->add_handler(log_handler);
}

void add_routes_handlers(std::shared_ptr<HTTPServer>& server) {
  std::vector<RouteDefinition> routes;

  routes.push_back(RouteDefinition("Status", "/status", "StatusPage"));
  routes.push_back(RouteDefinition("System", "/system", "SystemPage"));
  routes.push_back(RouteDefinition("Log", "/log", "LogPage"));
  routes.push_back(RouteDefinition("WiFi", "/wifi", "WiFiConfigPage"));
  routes.push_back(RouteDefinition("Signal K", "/signalk", "SignalKPage"));
  routes.push_back(
      RouteDefinition("Configuration", "/configuration", "ConfigurationPage"));
  routes.push_back(RouteDefinition("Control", "/control", "ControlPage"));

  // Pre-render the response
  JsonDocument json_doc;
  JsonArray routes_json = json_doc.to<JsonArray>();

  for (auto it = routes.begin(); it != routes.end(); ++it) {
    routes_json.add(it->as_json());
  }

  String response;

  serializeJson(routes_json, response);

  auto routes_handler = std::make_shared<HTTPRequestHandler>(
      1 << HTTP_GET, "/api/routes", [response](httpd_req_t* req) {
        httpd_resp_set_type(req, "application/json");
        httpd_resp_sendstr(req, response.c_str());
        return ESP_OK;
      });
  server->add_handler(routes_handler);

  // Find the root page

  StaticFileData* root_page = nullptr;
  for (int i = 0; i < sizeof(kFrontendFiles) / sizeof(StaticFileData); i++) {
    if (strcmp(kFrontendFiles[i].url, "/") == 0) {
      root_page = (StaticFileData*)&kFrontendFiles[i];
      break;
    }
  }
  if (root_page == nullptr) {
    ESP_LOGE(__FILENAME__, "Root page not found in kWebUIFiles");
    return;
  }

  // Add a handler for each route that returns the root page

  for (auto it = routes.begin(); it != routes.end(); ++it) {
    String path = it->get_path();
    auto route_handler = std::make_shared<HTTPRequestHandler>(
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

void add_base_app_http_command_handlers(std::shared_ptr<HTTPServer>& server) {
  add_http_reset_handler(server);
  add_http_restart_handler(server);
  add_http_info_handler(server);
  add_http_log_handler(server);
  add_routes_handlers(server);
}

}  // namespace sensesp
