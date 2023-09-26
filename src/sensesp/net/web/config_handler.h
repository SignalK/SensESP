#ifndef SENSESP_NET_HTTP_CONFIG_HANDLER_H_
#define SENSESP_NET_HTTP_CONFIG_HANDLER_H_

#include <string.h>

#include "ArduinoJson.h"
#include "sensesp/net/http_server.h"
#include "sensesp/system/configurable.h"

namespace sensesp {

/**
 * @brief Handle HTTP requests to /config.
 *
 * This class handles HTTP requests to /config. It is used by the HTTPServer
 * to provide a RESTful API for configuring Configurable objects.
 *
 */
class HTTPConfigHandler : public HTTPServerHandler {
 public:
  HTTPConfigHandler() : HTTPServerHandler(){};
  virtual void set_handler(HTTPServer* server) override {
    // handler for GET /config
    const httpd_uri_t config_get_handler = {
        .uri = "/config*",
        .method = HTTP_GET,
        .handler =
            [](httpd_req_t* req) {
              return call_member_handler(req,
                                         &HTTPConfigHandler::handle_config_get);
            },
        .user_ctx = this};
    server->register_handler(&config_get_handler);

    // handler for PUT /config
    const httpd_uri_t config_put_handler = {
        .uri = "/config/*",
        .method = HTTP_PUT,
        .handler =
            [](httpd_req_t* req) {
              return call_member_handler(req,
                                         &HTTPConfigHandler::handle_config_put);
            },
        .user_ctx = (void*)this};
    server->register_handler(&config_put_handler);
  };

 protected:
  /**
   * @brief Send a list of configurable objects to the client.
   *
   * @param req
   * @return esp_err_t
   */
  esp_err_t handle_config_list(httpd_req_t* req) {
    auto output_buffer_size = 200 * configurables.size();
    DynamicJsonDocument json_doc(output_buffer_size);
    JsonArray arr = json_doc.createNestedArray("keys");
    for (auto it = configurables.begin(); it != configurables.end(); ++it) {
      arr.add(it->first);
    }
    String response;
    serializeJson(json_doc, response);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, response.c_str());
    return ESP_OK;
  }

  /**
   * @brief Send the configuration of a configurable object to the client.
   *
   * @param req
   * @return esp_err_t
   */
  esp_err_t handle_config_get(httpd_req_t* req) {
    // get the URL tail after /config
    String url_tail = String(req->uri).substring(7);
    char url_tail_cstr[url_tail.length() + 1];
    urldecode2(url_tail_cstr, url_tail.c_str());
    url_tail = String(url_tail_cstr);
    if (url_tail.length() == 0 || url_tail == "/") {
      // return a list of all Configurable objects
      return handle_config_list(req);
    }

    // find the Configurable object with the matching config_path
    std::map<String, Configurable*>::iterator it = configurables.find(url_tail);
    if (it == configurables.end()) {
      httpd_resp_send_err(req, HTTPD_404_NOT_FOUND,
                          "No Configurable found with that path");
      return ESP_FAIL;
    }
    Configurable* confable = it->second;

    // get the configuration
    DynamicJsonDocument doc(2048);
    String response;
    JsonObject config = doc.createNestedObject("config");
    confable->get_configuration(config);
    doc["schema"] = serialized(confable->get_config_schema());
    doc["description"] = confable->get_description();
    serializeJson(doc, response);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, response.c_str());
    return ESP_OK;
  }

  /**
   * @brief Set the configuration of a configurable object.
   *
   * @param req
   * @return esp_err_t
   */
  esp_err_t handle_config_put(httpd_req_t* req) {
    // check that the content type is JSON
    if (get_content_type(req) != "application/json") {
      httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                          "application/json content type expected");
      return ESP_FAIL;
    }

    // get the URL tail after /config
    String url_tail = String(req->uri).substring(7);
    if (url_tail.length() == 0) {
      httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                          "No configuration path specified");
      return ESP_FAIL;
    }

    // find the Configurable object with the matching config_path
    std::map<String, Configurable*>::iterator it = configurables.find(url_tail);
    if (it == configurables.end()) {
      httpd_resp_send_err(req, HTTPD_404_NOT_FOUND,
                          "No Configurable found with that path");
      return ESP_FAIL;
    }
    Configurable* confable = it->second;

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
    DynamicJsonDocument doc(1024);
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
  }
};

}  // namespace sensesp

#endif  // SENSESP_NET_HTTP_CONFIG_HANDLER_H_
