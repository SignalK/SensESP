#ifndef SENSESP_NET_HTTP_STATIC_FILE_HANDLER_H_
#define SENSESP_NET_HTTP_STATIC_FILE_HANDLER_H_

#include <esp_http_server.h>

#include "ArduinoJson.h"
#include "sensesp/net/http_server.h"
#include "static_file_data.h"

namespace sensesp {

/**
 * @brief Provide handlers for static web content.
 *
 */
void add_static_file_handlers(std::shared_ptr<HTTPServer> server);

}  // namespace sensesp

#endif  // SENSESP_NET_HTTP_STATIC_PAGE_HANDLER_H_
