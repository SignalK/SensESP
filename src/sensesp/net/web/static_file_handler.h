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
class HTTPStaticFileHandler : public HTTPRequestHandler {
 public:
  HTTPStaticFileHandler(const StaticFileData* page_data)
      : HTTPRequestHandler(), pages_{page_data} {}

  virtual void set_handler(HTTPServer* server) override;

  static esp_err_t string_handler(httpd_req_t* req);
 protected:
  const StaticFileData* pages_;

};

}  // namespace sensesp

#endif  // SENSESP_NET_HTTP_STATIC_PAGE_HANDLER_H_
