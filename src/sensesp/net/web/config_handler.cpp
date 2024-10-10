#include "sensesp.h"

#include "config_handler.h"

#include <memory>

#include "sensesp/ui/config_item.h"

namespace sensesp {

bool get_item_data(JsonDocument& doc,
                   const std::shared_ptr<ConfigItemBase>& item) {
  JsonObject obj = doc.to<JsonObject>();

  String str;
  serializeJson(doc, str);

  obj["path"] = item->get_config_path();
  obj["title"] = item->get_title();
  obj["description"] = item->get_description();
  obj["requires_restart"] = item->requires_restart();
  obj["schema"] = serialized(item->get_config_schema());

  item->refresh();

  JsonObject config = obj["config"].to<JsonObject>();
  bool result = item->to_json(config);

  serializeJson(obj, str);

  if (doc.overflowed()) {
    ESP_LOGE("ConfigHandler", "JSON document overflowed");
    return false;
  }

  return result;
}

esp_err_t handle_config_item_list(httpd_req_t* req) {
  ESP_LOGI("ConfigHandler", "GET request to URL %s", req->uri);
  String url = String(req->uri);
  String query = "";
  if (url.indexOf('?') != -1) {
    query = url.substring(url.indexOf('?') + 1);
  }

  bool cards_only = false;

  // If query is "cards", return only the cards
  if (query == "cards") {
    cards_only = true;
  } else if (query != "") {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid query");
    return ESP_FAIL;
  }

  JsonDocument json_doc;
  JsonArray arr = json_doc.to<JsonArray>();

  auto config_items = ConfigItemBase::get_config_items();

  for (auto it = config_items->begin(); it != config_items->end(); ++it) {
    if (cards_only &&
        ((*it)->get_config_schema() == "null" ||
         (*it)->get_config_schema() == "{}" || (*it)->get_title() == "")) {
      continue;
    }
    const String& path = (*it)->get_config_path();
    if (path == "") {
      continue;
    }
    auto obj = arr.add(path);
  }

  String response;
  serializeJson(json_doc, response);
  httpd_resp_set_type(req, "application/json");
  httpd_resp_sendstr(req, response.c_str());
  return ESP_OK;
}

void add_config_list_handler(std::shared_ptr<HTTPServer>& server) {
  auto handler = std::make_shared<HTTPRequestHandler>(
      1 << HTTP_GET, "/api/config", handle_config_item_list);
  server->add_handler(handler);
}

void add_config_get_handler(std::shared_ptr<HTTPServer>& server) {
  auto handler = std::make_shared<HTTPRequestHandler>(
      1 << HTTP_GET, "/api/config/*", [](httpd_req_t* req) {
        ESP_LOGD("ConfigHandler", "GET request to URL %s", req->uri);
        String url_tail = String(req->uri).substring(11);
        String path;
        String query = "";
        if (url_tail.indexOf('?') != -1) {
          path = url_tail.substring(0, url_tail.indexOf('?'));
          query = url_tail.substring(url_tail.indexOf('?') + 1);
        } else {
          path = url_tail;
        }
        char path_cstr[path.length() + 1];
        urldecode2(path_cstr, path.c_str());
        url_tail = String(path_cstr);

        if (path.length() == 0) {
          // return a list of all ConfigItemT objects
          return handle_config_item_list(req);
        }

        // find the config item object with the matching config_path
        auto config_item = ConfigItemBase::get_config_item(url_tail);
        if (config_item == nullptr) {
          httpd_resp_send_err(req, HTTPD_404_NOT_FOUND,
                              "No ConfigItem found with that path");
          return ESP_FAIL;
        }

        JsonDocument doc;

        get_item_data(doc, config_item);

        String response;
        serializeJson(doc, response);
        httpd_resp_set_type(req, "application/json");
        httpd_resp_sendstr(req, response.c_str());
        return ESP_OK;
      });
  server->add_handler(handler);
}

void add_config_put_handler(std::shared_ptr<HTTPServer>& server) {
  auto handler = std::make_shared<HTTPRequestHandler>(
      1 << HTTP_PUT, "/api/config/*",
      [](httpd_req_t* req) {  // check that the content type is JSON
        ESP_LOGI(__FILENAME__, "PUT request to URL %s", req->uri);
        if (get_content_type(req) != "application/json") {
          httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                              "application/json content type expected");
          return ESP_FAIL;
        }

        // get the URL tail after /api/config
        String url_tail = String(req->uri).substring(11);
        if (url_tail.length() == 0) {
          httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                              "No configuration path specified");
          return ESP_FAIL;
        }

        // urldecode the URL tail
        char url_tail_cstr[url_tail.length() + 1];
        urldecode2(url_tail_cstr, url_tail.c_str());
        url_tail = String(url_tail_cstr);

        // find the ConfigItemT object with the matching config_path
        auto config_item = ConfigItemBase::get_config_item(url_tail);
        if (config_item == nullptr) {
          httpd_resp_send_err(req, HTTPD_404_NOT_FOUND,
                              "No Configurable found with that path");
          return ESP_FAIL;
        }

        // receive the payload
        size_t payload_len = req->content_len;
        char* payload = new char[payload_len + 1];
        int ret = httpd_req_recv(req, payload, payload_len);
        if (ret <= 0) {
          httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                              "Error receiving payload");
          return ESP_FAIL;
        }
        payload[payload_len] = '\0';

        ESP_LOGV("ConfigHandler", "Received payload: %s", payload);

        // parse the content as JSON
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);
        delete[] payload;
        if (error) {
          ESP_LOGE("ConfigHandler", "Error parsing JSON payload: %s",
                   error.c_str());
          httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                              "Error parsing JSON payload");
          return ESP_FAIL;
        }

        String response;
        bool result = config_item->from_json(doc.as<JsonObject>());
        if (!result) {
          ESP_LOGE("ConfigHandler", "Error applying JSON payload");
          httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                              "Invalid JSON payload");
          return ESP_FAIL;
        }
        config_item->save();
        response = "{\"status\":\"ok\"}";

        httpd_resp_set_type(req, "application/json");
        httpd_resp_sendstr(req, response.c_str());
        return ESP_OK;

      });
  server->add_handler(handler);
}

void add_config_handlers(std::shared_ptr<HTTPServer>& server) {
  add_config_list_handler(server);
  add_config_get_handler(server);
  add_config_put_handler(server);
}

}  // namespace sensesp
