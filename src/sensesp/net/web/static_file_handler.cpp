#include "static_file_handler.h"

namespace sensesp {

void HTTPStaticFileHandler::set_handler(HTTPServer* server) {
    for (int i = 0; pages_[i].url != nullptr; i++) {
      const StaticFileData& page = pages_[i];
      httpd_uri_t handler{.uri = page.url,
                          .method = HTTP_GET,
                          .handler =
                              [](httpd_req_t* req) {
                                return call_static_handler(
                                    req,
                                    &HTTPStaticFileHandler::string_handler);
                              },
                          .user_ctx = (void*)&page};
      server->register_handler(&handler);
    }
  };

  esp_err_t HTTPStaticFileHandler::string_handler(httpd_req_t* req) {
    const StaticFileData* page = (const StaticFileData*)req->user_ctx;
    httpd_resp_set_type(req, page->content_type);
    if (page->content_encoding != nullptr) {
      httpd_resp_set_hdr(req, kContentEncoding, page->content_encoding);
    }
    httpd_resp_send(req, page->content, page->content_length);
    return ESP_OK;
  }


}  // namespace sensesp
