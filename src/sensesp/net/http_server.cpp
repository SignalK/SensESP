#include "sensesp/net/http_server.h"

#include <functional>
#include <list>

#include "sensesp.h"

namespace sensesp {

std::list<HTTPServerHandler *> HTTPServerHandler::handlers_;

// from:
// https://stackoverflow.com/questions/2673207/c-c-url-decode-library/2766963
void urldecode2(char* dst, const char* src) {
  char a, b;
  while (*src) {
    if ((*src == '%') && ((a = src[1]) && (b = src[2])) &&
        (isxdigit(a) && isxdigit(b))) {
      if (a >= 'a') a -= 'a' - 'A';
      if (a >= 'A')
        a -= ('A' - 10);
      else
        a -= '0';
      if (b >= 'a') b -= 'a' - 'A';
      if (b >= 'A')
        b -= ('A' - 10);
      else
        b -= '0';
      *dst++ = 16 * a + b;
      src += 3;
    } else if (*src == '+') {
      *dst++ = ' ';
      src++;
    } else {
      *dst++ = *src++;
    }
  }
  *dst++ = '\0';
}

HTTPServer* HTTPServer::singleton_ = nullptr;

  void HTTPServer::register_handler(const httpd_uri_t* uri_handler) {
    esp_err_t error = httpd_register_uri_handler(server_, uri_handler);
    if (error != ESP_OK) {
      debugE("Error registering URI handler for %s: %s", uri_handler->uri,
             esp_err_to_name(error));
    }
  };


esp_err_t authenticate_request(HTTPAuthenticator* auth,
                               std::function<esp_err_t(httpd_req_t*)> handler,
                               httpd_req_t* req) {
  bool continue_;
  if (auth == nullptr) {
    continue_ = true;
  } else {
    continue_ = auth->authenticate_request(req);
  }
  if (continue_) {  // authentication successful
    return handler(req);
  } else {
    return ESP_OK;  // Even though the authentication failed, request handling
                    // was successful.
  }
}

esp_err_t call_static_handler(httpd_req_t* req,
                              esp_err_t (*handler)(httpd_req_t*)) {
  HTTPServer* server = HTTPServer::get_server();
  bool continue_;

  HTTPAuthenticator* auth = server->authenticator_;
  return authenticate_request(auth, handler, req);
}

}  // namespace sensesp
