#include "sensesp.h"

#include "config_handler.h"

namespace sensesp {

const char* poll_status_strings[] = {
    "error",
    "ok",
    "pending",
};

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

        ESP_LOGV("ConfigHandler", "URL parts: %s, %s", path.c_str(),
                 query.c_str());

        if (path.length() == 0) {
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

        JsonDocument doc;
        JsonObject config;
        config = doc["config"].to<JsonObject>();

        // Should return a JSON object with the following keys:
        // - status: "ok", "pending" or "error"
        // - config, optional: the configuration object
        // - schema, optional: the configuration schema
        // - description, optional: the configuration description

        ConfigurableResult status = ConfigurableResult::kOk;

        if (confable->is_async()) {
          if (query.startsWith("poll_get_result")) {
            // Poll the status of a previous async GET (get) request
            status = confable->poll_get_result(config);
            if (status == ConfigurableResult::kPending) {
              // Set result code 202
              httpd_resp_set_status(req, "202 Accepted");
            }
          } else
          if (query.startsWith("poll_put_result")) {
            // Poll the status of a previous async PUT (set) request
            status = confable->poll_set_result();
            if (status == ConfigurableResult::kPending) {
              // Set result code 202
              httpd_resp_set_status(req, "202 Accepted");
            }
          } else {
            // Initiate an async GET (get) request
            status = confable->async_get_configuration();
            if (status == ConfigurableResult::kOk) {
              // Return the resulting config object immediately
              confable->poll_get_result(config);
            } else {
              // Set result code 202
              httpd_resp_set_status(req, "202 Accepted");
            }
            doc["schema"] = serialized(confable->get_config_schema());
            doc["description"] = confable->get_description();
          }
          doc["status"] = poll_status_strings[static_cast<int>(status)];
        } else {
          // Synchronous GET (get) request
          if (query.startsWith("poll")) {
            // Polling is not supported for synchronous requests
            httpd_resp_send_err(
                req, HTTPD_400_BAD_REQUEST,
                "Polling is not supported for synchronous requests");
          }

          doc["status"] =
              poll_status_strings[static_cast<int>(ConfigurableResult::kOk)];

          doc["schema"] = serialized(confable->get_config_schema());
          doc["description"] = confable->get_description();
        }

        confable->get_configuration(config);

        String response;
        serializeJson(doc, response);
        ESP_LOGV("ConfigHandler", "Response: %s", response.c_str());
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

        String response;
        if (confable->is_async()) {
          // Initiate an async PUT (set) request
          ConfigurableResult result =
              confable->async_set_configuration(doc.as<JsonObject>());
          if (result == ConfigurableResult::kError) {
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                                "Error setting configuration");
            return ESP_FAIL;
          }
          confable->save_configuration();
          String result_string = poll_status_strings[static_cast<int>(result)];
          response = "{\"status\":\"" + result_string + "\"}";
          // Set result code 202
          httpd_resp_set_status(req, "202 Accepted");
        } else {
          // Synchronous PUT (set) request
          bool result = confable->set_configuration(doc.as<JsonObject>());
          if (!result) {
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                                "Error setting configuration");
            return ESP_FAIL;
          }
          confable->save_configuration();
          response = "{\"status\":\"ok\"}";
        }

        httpd_resp_set_type(req, "application/json");
        httpd_resp_sendstr(req, response.c_str());
        return ESP_OK;

      }));
}

void add_config_handlers(HTTPServer* server) {
  add_config_list_handler(server);
  add_config_get_handler(server);
  add_config_put_handler(server);
}

}  // namespace sensesp
