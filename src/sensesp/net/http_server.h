#ifndef SENSESP_NET_HTTP_SERVER_H_
#define SENSESP_NET_HTTP_SERVER_H_

#include "sensesp.h"

#include <ESPmDNS.h>
#include <esp_http_server.h>
#include <functional>
#include <list>

#include "WiFi.h"
#include "sensesp/net/http_authenticator.h"
#include "sensesp/system/configurable.h"
#include "sensesp_base_app.h"

#ifndef HTTP_SERVER_STACK_SIZE
#define HTTP_SERVER_STACK_SIZE 8192
#endif

namespace sensesp {

#ifndef HTTP_DEFAULT_PORT
#define HTTP_DEFAULT_PORT 80
#endif

#include <ctype.h>
#include <stdlib.h>

void urldecode2(char* dst, const char* src);
String get_content_type(httpd_req_t* req);
esp_err_t call_request_dispatcher(httpd_req_t* req);

class HTTPServer;

/**
 * @brief HTTP request handler storage class.
 *
 */
class HTTPRequestHandler {
 public:
  HTTPRequestHandler(uint32_t method_mask, String match_uri,
                     std::function<esp_err_t(httpd_req_t*)> handler_func)
      : method_mask_(method_mask),
        match_uri_(match_uri),
        handler_func_(handler_func) {}

  const uint32_t method_mask_;
  const String match_uri_;

  esp_err_t call(httpd_req_t* req) { return this->handler_func_(req); }

 protected:
  const std::function<esp_err_t(httpd_req_t*)> handler_func_;
};

/**
 * @brief HTTP server class wrapping the esp-idf http server.
 *
 */
class HTTPServer : public Configurable {
 public:
  HTTPServer(int port = HTTP_DEFAULT_PORT,
             const String& config_path = "/system/httpserver")
      : Configurable(config_path), config_(HTTPD_DEFAULT_CONFIG()) {
    config_.server_port = port;
    config_.stack_size = HTTP_SERVER_STACK_SIZE;
    config_.max_uri_handlers = 20;
    config_.uri_match_fn = httpd_uri_match_wildcard;
    String auth_realm_ = "Login required for " + SensESPBaseApp::get_hostname();
    load_configuration();
    if (auth_required_) {
      authenticator_ =
          new HTTPDigestAuthenticator(username_, password_, auth_realm_);
    }
    if (singleton_ == nullptr) {
      singleton_ = this;
      ESP_LOGD(__FILENAME__, "HTTPServer instance created");
    } else {
      ESP_LOGE(__FILENAME__, "Only one HTTPServer instance is allowed");
      return;
    }
    reactesp::ReactESP::app->onDelay(0, [this]() {
      esp_err_t error = httpd_start(&server_, &config_);
      if (error != ESP_OK) {
        ESP_LOGE(__FILENAME__, "Error starting HTTP server: %s",
                 esp_err_to_name(error));
      } else {
        ESP_LOGI(__FILENAME__, "HTTP server started");
      }

      // register the request dispatcher for all methods and all URIs
      httpd_uri_t uri = {
          .uri = "/*",
          .method = HTTP_GET,
          .handler = call_request_dispatcher,
          .user_ctx = this,
          .is_websocket = false,
      };
      httpd_register_uri_handler(server_, &uri);
      uri.method = HTTP_HEAD;
      httpd_register_uri_handler(server_, &uri);
      uri.method = HTTP_POST;
      httpd_register_uri_handler(server_, &uri);
      uri.method = HTTP_PUT;
      httpd_register_uri_handler(server_, &uri);
      uri.method = HTTP_DELETE;
      httpd_register_uri_handler(server_, &uri);

      // announce the server over mDNS
      MDNS.addService("http", "tcp", 80);
    });
  };
  ~HTTPServer() {}

  void stop() { httpd_stop(server_); }

  void set_auth_credentials(const String& username, const String& password,
                            bool auth_required = true) {
    username_ = username;
    password_ = password;
    auth_required_ = auth_required;
  }

  void set_captive_portal(bool captive_portal) {
    captive_portal_ = captive_portal;
  }

  virtual void get_configuration(JsonObject& config) override {
    config["auth_required"] = auth_required_;
    config["username"] = username_;
    config["password"] = password_;
  }

  virtual bool set_configuration(const JsonObject& config) override {
    if (config.containsKey("auth_required")) {
      auth_required_ = config["auth_required"];
    }
    if (config.containsKey("username")) {
      username_ = config["username"].as<String>();
    }
    if (config.containsKey("password")) {
      password_ = config["password"].as<String>();
    }
    return true;
  }

  void add_handler(HTTPRequestHandler* handler) {
    handlers_.push_back(handler);
  }

 protected:
  static HTTPServer* get_server() { return singleton_; }
  static HTTPServer* singleton_;
  bool captive_portal_ = false;
  httpd_handle_t server_ = nullptr;
  httpd_config_t config_;
  String username_;
  String password_;
  bool auth_required_ = false;
  HTTPAuthenticator* authenticator_ = nullptr;

  bool authenticate_request(HTTPAuthenticator* auth,
                            std::function<esp_err_t(httpd_req_t*)> handler,
                            httpd_req_t* req);

  /**
   * @brief Dispatcher method that captures all requests and forwards them to
   * the appropriate handlers.
   *
   * @param req
   * @return esp_err_t
   */
  esp_err_t dispatch_request(httpd_req_t* req);

  /**
   * @brief Check if the request is for the captive portal and handle it if it
   *
   * @param req
   * @return true Request was handled
   * @return false Request was not handled
   */
  bool handle_captive_portal(httpd_req_t* req);

  std::list<HTTPRequestHandler*> handlers_;

  friend esp_err_t call_request_dispatcher(httpd_req_t* req);
};

}  // namespace sensesp

#endif  // SENSESP_NET_HTTP_SERVER_H_
