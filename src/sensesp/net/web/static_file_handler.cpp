#include "static_file_handler.h"

#include "autogen/web_ui_files.h"

namespace sensesp {

void add_static_file_handlers(HTTPServer* server) {
  for (int i = 0; kWebUIFiles[i].url != nullptr; i++) {
    const StaticFileData& data = kWebUIFiles[i];
    HTTPRequestHandler* handler = new HTTPRequestHandler(
        1 << HTTP_GET, kWebUIFiles[i].url, [data](httpd_req_t* req) {
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
