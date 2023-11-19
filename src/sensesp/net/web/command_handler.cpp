#include "command_handler.h"

namespace sensesp {

void add_http_command_handlers(HTTPServer* server) {
  (new HTTPResetHandler())->set_handler(server);
  (new HTTPRestartHandler())->set_handler(server);
  (new HTTPInfoHandler())->set_handler(server);
  (new HTTPRoutesHandler())->set_handler(server);
}

}  // namespace sensesp
