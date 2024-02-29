#include "config_handler.h"

#include "sensesp.h"

namespace sensesp {

esp_err_t handle_config_list(httpd_req_t* req) {
  JsonDocument json_doc;
  JsonArray arr = json_doc["keys"].to<JsonArray>();

  std::vector<Configurable*> configurables = Configurable::get_configurables();

  for (auto it = configurables.begin(); it != configurables.end(); ++it) {
    if ((*it)->get_config_schema() != "{}") {
      arr.add((*it)->config_path_);
    }
  }

  String response;
  serializeJson(json_doc, response);
  httpd_resp_set_type(req, "application/json");
  httpd_resp_sendstr(req, response.c_str());
  return ESP_OK;
}

void add_config_list_handler(HTTPServer* server) {
  server->add_handler(
      new HTTPRequestHandler(1 << HTTP_GET, "/api/config", handle_config_list));
}

void add_config_get_handler(HTTPServer* server) {
  server->add_handler(new HTTPRequestHandler(
      1 << HTTP_GET, "/api/config/*", [](httpd_req_t* req) {
        String url_tail = String(req->uri).substring(11);
        char url_tail_cstr[url_tail.length() + 1];
        urldecode2(url_tail_cstr, url_tail.c_str());
        url_tail = String(url_tail_cstr);
        if (url_tail.length() == 0) {
          // return a list of all Configurable objects
          return handle_config_list(req);
        }

        // find the Configurable object with the matching config_path
        Configurable* confable = Configurable::get_configurable(url_tail);
        if (confable == nullptr) {
          httpd_resp_send_err(req, HTTPD_404_NOT_FOUND,
                              "No Configurable found with that path");
          return ESP_FAIL;
        }

        // get the configuration
        JsonDocument doc;
        String response;
        JsonObject config = doc["config"].to<JsonObject>();
        confable->get_configuration(config);
        doc["schema"] = serialized(confable->get_config_schema());
        doc["description"] = confable->get_description();
        serializeJson(doc, response);
        httpd_resp_set_type(req, "application/json");
        httpd_resp_sendstr(req, response.c_str());
        return ESP_OK;
      }));
}

void add_config_put_handler(HTTPServer* server) {
  server->add_handler(new HTTPRequestHandler(
      1 << HTTP_PUT, "/api/config/*",
      [](httpd_req_t* req) {  // check that the content type is JSON
        debugI("Handling PUT request to URL %s", req->uri);
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

        // find the Configurable object with the matching config_path
        Configurable* confable = Configurable::get_configurable(url_tail);
        if (confable == nullptr) {
          httpd_resp_send_err(req, HTTPD_404_NOT_FOUND,
                              "No Configurable found with that path");
          return ESP_FAIL;
        }

        // receive the content
        size_t content_len = req->content_len;
        char* content = new char[content_len + 1];
        int ret = httpd_req_recv(req, content, content_len);
        if (ret <= 0) {
          httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                              "Error receiving content");
          return ESP_FAIL;
        }
        content[content_len] = '\0';

        // parse the content as JSON
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, content);
        if (error) {
          httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                              "Error parsing JSON content");
          return ESP_FAIL;
        }

        // set the configuration
        if (!confable->set_configuration(doc.as<JsonObject>())) {
          httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                              "Error setting configuration");
          return ESP_FAIL;
        }

        // save the configuration
        confable->save_configuration();
        httpd_resp_sendstr(req, "OK");
        return ESP_OK;
      }));
}

void add_config_handlers(HTTPServer* server) {
  add_config_list_handler(server);
  add_config_get_handler(server);
  add_config_put_handler(server);
}

}  // namespace sensesp
