#include "sensesp/net/http_server.h"

#include <lwip/sockets.h>

#include <functional>
#include <list>

#include "sensesp.h"

namespace sensesp {

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

esp_err_t HTTPServer::dispatch_request(httpd_req_t* req) {
  debugI("Handling request: %s", req->uri);

  if (captive_portal_) {
    bool captured;
    captured = handle_captive_portal(req);
    if (captured) {
      return ESP_OK;
    }
  }

  if (auth_required_) {
    bool success;
    success =
        authenticate_request(authenticator_, call_request_dispatcher, req);
    if (!success) {
      // Authentication failed; do not continue but return success.
      // The client already received a 401 response.
      return ESP_OK;
    }
  }

  // dispatch request to the appropriate handler

  String uri = req->uri;

  // decode the uri
  char decoded_uri[uri.length() + 1];
  urldecode2(decoded_uri, uri.c_str());
  String decoded_uri_str = String(decoded_uri);

  // Drop the query part of the URI
  int query_pos = decoded_uri_str.indexOf('?');
  if (query_pos != -1) {
    decoded_uri_str = decoded_uri_str.substring(0, query_pos);
  }

  for (auto handler : handlers_) {
    String match_uri = handler->match_uri_;

    // Check if the match uri ends with a wildcard
    if (match_uri.endsWith("*")) {
      // Remove the wildcard from the match uri
      match_uri = match_uri.substring(0, match_uri.length() - 1);
      // Check if the request uri starts with the match uri
      if (decoded_uri_str.startsWith(match_uri)) {
        if (handler->method_mask_ & (1 << req->method)) {
          return handler->call(req);
        }
      }
    } else if (handler->match_uri_ == req->uri) {
      if (handler->method_mask_ & (1 << req->method)) {
        return handler->call(req);
      }
    }
  }

  return httpd_resp_send_404(req);
}

bool HTTPServer::handle_captive_portal(httpd_req_t* req) {
  // Get HTTP Host header
  char host[100];
  if (httpd_req_get_hdr_value_str(req, "Host", host, sizeof(host)) != ESP_OK) {
    return false;
  }
  String host_hdr = host;
  // Drop port number
  int pos = host_hdr.indexOf(':');
  if (pos != -1) {
    host_hdr = host_hdr.substring(0, pos);
  }

  // Get the HTTP request socket local IP address
  int sockfd = httpd_req_to_sockfd(req);
  char ipstr[INET6_ADDRSTRLEN];
  struct sockaddr_in6 addr;  // esp_http_server uses IPv6 addressing
  socklen_t addr_size = sizeof(addr);
  if (getsockname(sockfd, (struct sockaddr*)&addr, &addr_size) < 0) {
    debugE("Error getting client IP");
    return false;
  }
  inet_ntop(AF_INET, &addr.sin6_addr.un.u32_addr[3], ipstr, sizeof(ipstr));
  debugD("Local IP address: %s", ipstr);

  String ap_ip = WiFi.softAPIP().toString();

  // We should only apply the captive portal to the soft AP IP address
  if (ap_ip != ipstr) {
    debugD("Not a captive portal request");
    return false;
  }

  // Check if the host matches our soft AP IP address
  if (host_hdr != ap_ip) {
    // Redirect the client to the captive portal url /wifi
    httpd_resp_set_status(req, "302 Found");
    String destination = String("http://") + ap_ip + "/wifi";
    httpd_resp_set_hdr(req, "Location", destination.c_str());
    httpd_resp_sendstr(req, "Redirecting to captive portal");
    return true;
  }
  return false;
}

bool HTTPServer::authenticate_request(
    HTTPAuthenticator* auth, std::function<esp_err_t(httpd_req_t*)> handler,
    httpd_req_t* req) {
  bool continue_ = false;
  if (auth == nullptr) {
    continue_ = true;
  } else {
    continue_ = auth->authenticate_request(req);
  }

  return continue_;
}

esp_err_t call_request_dispatcher(httpd_req_t* req) {
  HTTPServer* server = HTTPServer::get_server();
  bool continue_;

  return server->dispatch_request(req);
}

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

}  // namespace sensesp
