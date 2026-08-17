#include "button_handler.h"

#include "sensesp/net/web/base_command_handler.h"
#include "sensesp/ui/ui_button.h"

namespace sensesp {

void add_button_list_handler(std::shared_ptr<HTTPServer>& server) {
  auto handler = std::make_shared<HTTPRequestHandler>(
      1 << HTTP_GET, "/api/buttons", [](httpd_req_t* req) {
        JsonDocument json_doc;
        JsonArray arr = json_doc.to<JsonArray>();

        for (const auto& entry : UIButton::get_ui_buttons()) {
          JsonObject obj = arr.add<JsonObject>();
          obj["name"] = entry.second->get_name();
          obj["title"] = entry.second->get_title();
          obj["must_confirm"] = entry.second->get_must_confirm();
        }

        String response;
        serializeJson(json_doc, response);
        httpd_resp_set_type(req, "application/json");
        httpd_resp_sendstr(req, response.c_str());
        return ESP_OK;
      });
  server->add_handler(handler);
}

void add_button_click_handler(std::shared_ptr<HTTPServer>& server) {
  auto handler = std::make_shared<HTTPRequestHandler>(
      1 << HTTP_POST, "/api/buttons/*", [](httpd_req_t* req) {
        if (!check_origin(req)) {
          return ESP_FAIL;
        }

        // strlen("/api/buttons/") == 13
        String url_tail = String(req->uri).substring(13);
        int query_start = url_tail.indexOf('?');
        if (query_start != -1) {
          url_tail = url_tail.substring(0, query_start);
        }
        char name_cstr[url_tail.length() + 1];
        urldecode2(name_cstr, url_tail.c_str());
        String name(name_cstr);

        const auto& buttons = UIButton::get_ui_buttons();
        auto it = buttons.find(name);
        if (it == buttons.end()) {
          httpd_resp_send_err(req, HTTPD_404_NOT_FOUND,
                              "No button found with that name");
          return ESP_FAIL;
        }

        it->second->notify();

        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, "{\"status\":\"ok\"}", 0);
        return ESP_OK;
      });
  server->add_handler(handler);
}

void add_button_handlers(std::shared_ptr<HTTPServer>& server) {
  add_button_list_handler(server);
  add_button_click_handler(server);
}

}  // namespace sensesp
