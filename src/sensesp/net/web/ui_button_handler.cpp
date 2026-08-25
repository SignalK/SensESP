#include "sensesp.h"

#include "ui_button_handler.h"

#include "sensesp/net/web/base_command_handler.h"
#include "sensesp/ui/ui_button.h"

namespace sensesp {

namespace {

// Names longer than this cannot be triggered over HTTP; the limit is
// documented on UIButton::add(). Percent-encoding expands a character to
// up to three bytes, so the encoded tail is bounded at three times the
// limit before decoding.
constexpr size_t kMaxButtonNameLength = 64;
constexpr size_t kMaxEncodedNameLength = 3 * kMaxButtonNameLength;

void add_button_list_handler(std::shared_ptr<HTTPServer>& server) {
  auto handler = std::make_shared<HTTPRequestHandler>(
      1 << HTTP_GET, "/api/buttons", [](httpd_req_t* req) {
        JsonDocument json_doc;
        JsonArray arr = json_doc.to<JsonArray>();

        for (const auto& entry : UIButton::get_ui_buttons()) {
          JsonObject obj = arr.add<JsonObject>();
          obj["name"] = entry.second->get_name();
          obj["title"] = entry.second->get_title();
          obj["mustConfirm"] = entry.second->get_must_confirm();
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
        if (url_tail.length() > kMaxEncodedNameLength) {
          httpd_resp_send_err(req, HTTPD_404_NOT_FOUND,
                              "No button found with that name");
          return ESP_FAIL;
        }
        char name_cstr[kMaxEncodedNameLength + 1];
        urldecode2(name_cstr, url_tail.c_str());
        String name(name_cstr);
        if (name.length() > kMaxButtonNameLength) {
          httpd_resp_send_err(req, HTTPD_404_NOT_FOUND,
                              "No button found with that name");
          return ESP_FAIL;
        }

        const auto& buttons = UIButton::get_ui_buttons();
        auto it = buttons.find(name);
        if (it == buttons.end()) {
          httpd_resp_send_err(req, HTTPD_404_NOT_FOUND,
                              "No button found with that name");
          return ESP_FAIL;
        }

        httpd_resp_set_type(req, "application/json");
        httpd_resp_sendstr(req, "{\"status\":\"ok\"}");

        // User callbacks belong on the event loop task: the httpd task stack
        // is not sized for them, and they may touch event-loop-owned state.
        // The shared_ptr copy keeps the button alive even if a callback
        // mutates the registry. A 200 means the click was accepted, as with
        // /api/device/reset and /api/device/restart.
        auto button = it->second;
        event_loop()->onDelay(0, [button]() { button->notify(); });
        return ESP_OK;
      });
  server->add_handler(handler);
}

}  // namespace

void add_button_handlers(std::shared_ptr<HTTPServer>& server) {
  add_button_list_handler(server);
  add_button_click_handler(server);
}

}  // namespace sensesp
