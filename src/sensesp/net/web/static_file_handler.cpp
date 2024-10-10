#include "static_file_handler.h"

#include "autogen/frontend_files.h"

namespace sensesp {

void add_static_file_handlers(std::shared_ptr<HTTPServer> server) {
  for (int i = 0; kFrontendFiles[i].url != nullptr; i++) {
    const StaticFileData& data = kFrontendFiles[i];
    HTTPRequestHandler* handler = new HTTPRequestHandler(
        1 << HTTP_GET, kFrontendFiles[i].url, [data](httpd_req_t* req) {
          httpd_resp_set_type(req, data.content_type);
          if (data.content_encoding != nullptr) {
            httpd_resp_set_hdr(req, kContentEncoding, data.content_encoding);
          }
          httpd_resp_send(req, data.content, data.content_length);
          return ESP_OK;
        });
    server->add_handler(handler);
  }
}

}  // namespace sensesp
