#ifndef SENSESP_NET_HTTP_APP_COMMAND_HANDLER_H_
#define SENSESP_NET_HTTP_APP_COMMAND_HANDLER_H_

#include <vector>

#include "ArduinoJson.h"
#include "sensesp/net/http_server.h"
#include "sensesp/system/configurable.h"
#include "sensesp_app.h"

namespace sensesp {

void add_app_http_command_handlers(HTTPServer* server);

}  // namespace sensesp

#endif
