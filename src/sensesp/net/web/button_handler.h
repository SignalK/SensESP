#ifndef SENSESP_NET_WEB_BUTTON_HANDLER_H_
#define SENSESP_NET_WEB_BUTTON_HANDLER_H_

#include <memory>

#include "sensesp/net/http_server.h"

namespace sensesp {

/**
 * @brief Handle HTTP requests to /api/buttons.
 *
 * Serves the UIButton registry (GET /api/buttons) and dispatches clicks
 * (POST /api/buttons/<name>) to the corresponding button's observers.
 */
void add_button_handlers(std::shared_ptr<HTTPServer>& server);

}  // namespace sensesp

#endif  // SENSESP_NET_WEB_BUTTON_HANDLER_H_
