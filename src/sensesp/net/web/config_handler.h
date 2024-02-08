#ifndef SENSESP_NET_HTTP_CONFIG_HANDLER_H_
#define SENSESP_NET_HTTP_CONFIG_HANDLER_H_

#include <string.h>

#include "ArduinoJson.h"
#include "sensesp/net/http_server.h"
#include "sensesp/system/configurable.h"

namespace sensesp {

/**
 * @brief Handle HTTP requests to /config.
 *
 * This class handles HTTP requests to /config. It is used by the HTTPServer
 * to provide a RESTful API for configuring Configurable objects.
 *
 */
void add_config_handlers(HTTPServer* server);

}  // namespace sensesp

#endif  // SENSESP_NET_HTTP_CONFIG_HANDLER_H_
