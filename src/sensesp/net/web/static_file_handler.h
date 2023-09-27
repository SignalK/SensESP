#ifndef SENSESP_NET_HTTP_STATIC_FILE_HANDLER_H_
#define SENSESP_NET_HTTP_STATIC_FILE_HANDLER_H_

#include <esp_http_server.h>

#include "ArduinoJson.h"
#include "sensesp/net/http_server.h"
#include "sensesp/system/configurable.h"
#include "static_file_data.h"

namespace sensesp {

/**
 * @brief Provide handlers for static web content.
 *
 */
class HTTPStaticFileHandler : public HTTPServerHandler {
 public:
  HTTPStaticFileHandler(const StaticFileData* page_data) : HTTPServerHandler() {
    pages_ = page_data;
  };
  virtual void set_handler(HTTPServer* server) override {
    for (int i = 0; pages_[i].url != nullptr; i++) {
      const StaticFileData& page = pages_[i];
      httpd_uri_t handler{.uri = page.url,
                          .method = HTTP_GET,
                          .handler = HTTPStaticFileHandler::string_handler,
                          .user_ctx = (void*)&page};
      server->register_handler(&handler);
    }
  };

 protected:
  const StaticFileData* pages_;

  static esp_err_t string_handler(httpd_req_t* req) {
    const StaticFileData* page = (const StaticFileData*)req->user_ctx;
    httpd_resp_set_type(req, page->content_type);
    if (page->content_encoding != nullptr) {
      httpd_resp_set_hdr(req, kContentEncoding, page->content_encoding);
    }
    httpd_resp_send(req, page->content, page->content_length);
    return ESP_OK;
  }
};

}  // namespace sensesp

#endif  // SENSESP_NET_HTTP_STATIC_PAGE_HANDLER_H_
