#ifndef SENSESP_NET_HTTP_SERVER_H_
#define SENSESP_NET_HTTP_SERVER_H_

#include <ESPmDNS.h>
#include <esp_http_server.h>

#include <list>

#include "WiFi.h"
#include "sensesp.h"
#include "sensesp/system/startable.h"

namespace sensesp {

#ifndef HTTP_DEFAULT_PORT
#define HTTP_DEFAULT_PORT 80
#endif

class HTTPServer;

#include <ctype.h>
#include <stdlib.h>

void urldecode2(char *dst, const char *src);

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
  virtual void set_handler(HTTPServer* server) = 0;

  static void register_handlers(HTTPServer* server) {
    for (auto& handler : handlers_) {
      handler->set_handler(server);
    }
  }

 protected:
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

 private:
  static std::list<HTTPServerHandler*> handlers_;
};

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
  return (static_cast<T*>(req->user_ctx)->*member)(req);
}

/**
 * @brief HTTP server class wrapping the esp-idf http server.
 *
 */
class HTTPServer : public Startable {
 public:
  HTTPServer(int port = HTTP_DEFAULT_PORT)
      : config_(HTTPD_DEFAULT_CONFIG()), Startable(50) {
    config_.server_port = port;
    config_.max_uri_handlers = 20;
    config_.uri_match_fn = httpd_uri_match_wildcard;
  };
  ~HTTPServer() {}

  virtual void start() override {
    esp_err_t error = httpd_start(&server_, &config_);
    if (error != ESP_OK) {
      debugE("Error starting HTTP server: %s", esp_err_to_name(error));
    } else {
      debugI("HTTP server started");
    }
    HTTPServerHandler::register_handlers(this);

    // announce the server over mDNS
    MDNS.addService("http", "tcp", 80);
  }
  void stop() { httpd_stop(server_); }

  void register_handler(const httpd_uri_t* uri_handler) {
    esp_err_t error = httpd_register_uri_handler(server_, uri_handler);
    if (error != ESP_OK) {
      debugE("Error registering URI handler for %s: %s", uri_handler->uri,
             esp_err_to_name(error));
    }
  };

 protected:
  httpd_handle_t server_ = nullptr;
  httpd_config_t config_;
};

}  // namespace sensesp

#endif  // SENSESP_NET_HTTP_SERVER_H_
