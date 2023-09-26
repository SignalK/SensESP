#ifndef SENSESP_NET_HTTP_STATIC_PAGE_HANDLER_H_
#define SENSESP_NET_HTTP_STATIC_PAGE_HANDLER_H_

#include <esp_http_server.h>

#include "ArduinoJson.h"
#include "sensesp/net/http_server.h"
#include "sensesp/system/configurable.h"
#include "static/css_bootstrap.h"
#include "static/index.h"
#include "static/js_jsoneditor.h"
#include "static/js_sensesp.h"

namespace sensesp {

const char kContentType[] = "Content-Type";
const char kContentEncoding[] = "Content-Encoding";
const char kContentEncodingGzip[] = "gzip";
const char kContentTypeHtml[] = "text/html";
const char kContentTypeCss[] = "text/css";
const char kContentTypeJs[] = "application/javascript";

struct PageData {
  const char* url;
  const char* content;
  const unsigned int content_length;
  const char* content_type;
  const char* content_encoding;
};

static PageData pages[] = {
    {"/", PAGE_index, sizeof(PAGE_index), kContentTypeHtml,
     kContentEncodingGzip},
    {"/css/bootstrap.min.css", PAGE_css_bootstrap, sizeof(PAGE_css_bootstrap),
     kContentTypeCss, kContentEncodingGzip},
    {"/js/jsoneditor.min.js", PAGE_js_jsoneditor, sizeof(PAGE_js_jsoneditor),
     kContentTypeHtml, kContentEncodingGzip},
    {"/js/sensesp.js", PAGE_js_sensesp, sizeof(PAGE_js_sensesp), kContentTypeJs,
     kContentEncodingGzip}};

/**
 * @brief Provide handlers for static web content.
 *
 */
class HTTPStaticPageHandler : public HTTPServerHandler {
 public:
  HTTPStaticPageHandler() : HTTPServerHandler(){};
  virtual void set_handler(HTTPServer* server) override {
    for (int i = 0; i < sizeof(pages) / sizeof(pages[0]); i++) {
      const PageData& page = pages[i];
      httpd_uri_t handler{.uri = page.url,
                          .method = HTTP_GET,
                          .handler = HTTPStaticPageHandler::string_handler,
                          .user_ctx = (void*)&page};
      server->register_handler(&handler);
    }
  };

 protected:
  static esp_err_t string_handler(httpd_req_t* req) {
    const PageData* page = (const PageData*)req->user_ctx;
    httpd_resp_set_type(req, page->content_type);
    if (page->content_encoding) {
      httpd_resp_set_hdr(req, kContentEncoding, page->content_encoding);
    }
    httpd_resp_send(req, page->content, page->content_length);
    return ESP_OK;
  }
};

}  // namespace sensesp

#endif  // SENSESP_NET_HTTP_STATIC_PAGE_HANDLER_H_
