#ifndef SENSESP_NET_HTTP_APP_COMMAND_HANDLER_H_
#define SENSESP_NET_HTTP_APP_COMMAND_HANDLER_H_

#include <memory>
#include <vector>

#include "ArduinoJson.h"
#include "sensesp/net/http_server.h"
#include "sensesp/net/networking.h"

namespace sensesp {

void add_app_http_command_handlers(std::shared_ptr<HTTPServer>& server,
                                   std::shared_ptr<Networking>& networking);

}  // namespace sensesp

#endif
