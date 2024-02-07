#ifndef SENSESP_NET_HTTP_SERVER_H_
#define SENSESP_NET_HTTP_SERVER_H_

#include <ESPmDNS.h>
#include <esp_http_server.h>

#include <functional>
#include <list>

#include "WiFi.h"
#include "sensesp.h"
#include "sensesp/net/http_authenticator.h"
#include "sensesp/system/configurable.h"
#include "sensesp_base_app.h"

namespace sensesp {

#ifndef HTTP_DEFAULT_PORT
#define HTTP_DEFAULT_PORT 80
#endif

#include <ctype.h>
#include <stdlib.h>

void urldecode2(char* dst, const char* src);

class HTTPServer;

/**
 * @brief Base class for HTTP server handlers.
 *
 * A class that inherits from HTTPServerHandler can be will have its
 * `set_handler` method called when the HTTP server starts. This method
 * should register the handler with the server.
 *
 */
class HTTPServerHandler {
 public:
  HTTPServerHandler() {
    // add this handler to the list of handlers
    handlers_.push_back(this);
  }

  static void register_handlers(HTTPServer* server) {
    for (auto& handler : handlers_) {
      handler->register_handler(server);
    }
  }

 protected:
  virtual void set_handler(HTTPServer* server) = 0;

  String get_content_type(httpd_req_t* req) {
    if (httpd_req_get_hdr_value_len(req, "Content-Type") != 16) {
      debugE("Invalid content type");
      return "";
    } else {
      char content_type[32];
      httpd_req_get_hdr_value_str(req, "Content-Type", content_type, 32);
      return String(content_type);
    }
  }

  void register_handler(HTTPServer* server) { this->set_handler(server); }

 private:
  static std::list<HTTPServerHandler*> handlers_;

  template <class T>
  friend esp_err_t call_member_handler(httpd_req_t* req,
                                       esp_err_t (T::*member)(httpd_req_t*));
};

/**
 * @brief HTTP server class wrapping the esp-idf http server.
 *
 */
class HTTPServer : public Configurable {
 public:
  HTTPServer(int port = HTTP_DEFAULT_PORT,
             String config_path = "/system/httpserver", String description = "",
             int sort_order = 0)
      : config_(HTTPD_DEFAULT_CONFIG()),
        Configurable(config_path, description, sort_order) {
    config_.server_port = port;
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
      debugD("HTTPServer instance created");
    } else {
      debugE("Only one HTTPServer instance is allowed");
      return;
    }
    ReactESP::app->onDelay(0, [this]() {
      esp_err_t error = httpd_start(&server_, &config_);
      if (error != ESP_OK) {
        debugE("Error starting HTTP server: %s", esp_err_to_name(error));
      } else {
        debugI("HTTP server started");
      }
      HTTPServerHandler::register_handlers(this);

      // announce the server over mDNS
      MDNS.addService("http", "tcp", 80);
    });
  };
  ~HTTPServer() {}

  void stop() { httpd_stop(server_); }

  void register_handler(const httpd_uri_t* uri_handler);

  void set_auth_credentials(const String& username, const String& password,
                            bool auth_required = true) {
    username_ = username;
    password_ = password;
    auth_required_ = auth_required;
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

 protected:
  static HTTPServer* get_server() { return singleton_; }
  static HTTPServer* singleton_;
  httpd_handle_t server_ = nullptr;
  httpd_config_t config_;
  String username_;
  String password_;
  bool auth_required_ = false;
  HTTPAuthenticator* authenticator_ = nullptr;

  template <class T>
  friend esp_err_t call_member_handler(httpd_req_t* req,
                                       esp_err_t (T::*member)(httpd_req_t*));
  friend esp_err_t call_static_handler(httpd_req_t* req,
                                       esp_err_t (*handler)(httpd_req_t*));
};

esp_err_t authenticate_request(HTTPAuthenticator* auth,
                               std::function<esp_err_t(httpd_req_t*)> handler,
                               httpd_req_t* req);

/**
 * @brief Call a member function of class T as a request handler.
 *
 * The object pointer is cast from the user_ctx of the request.
 *
 * @tparam T
 * @param req
 * @param member
 * @return esp_err_t
 */
template <typename T>
esp_err_t call_member_handler(httpd_req_t* req,
                              esp_err_t (T::*member)(httpd_req_t*)) {
  HTTPServer* server = HTTPServer::get_server();
  auto* handler = static_cast<T*>(req->user_ctx);
  bool continue_;

  std::function<esp_err_t(httpd_req_t*)> memberfunc =
      [handler, member](httpd_req_t* req) { return (handler->*member)(req); };

  HTTPAuthenticator* auth = server->authenticator_;
  return authenticate_request(auth, memberfunc, req);
}

/**
 * @brief Call a static function as a request handler.
 *
 * @param req
 * @param handler
 * @return esp_err_t
 */
esp_err_t call_static_handler(httpd_req_t* req,
                              esp_err_t (*handler)(httpd_req_t*));

}  // namespace sensesp

#endif  // SENSESP_NET_HTTP_SERVER_H_
