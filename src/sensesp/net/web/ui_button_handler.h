#ifndef SENSESP_NET_WEB_UI_BUTTON_HANDLER_H_
#define SENSESP_NET_WEB_UI_BUTTON_HANDLER_H_

#include <memory>

#include "sensesp/net/http_server.h"

namespace sensesp {

/**
 * @brief Handle HTTP requests to /api/buttons.
 *
 * Serves the UIButton registry (GET /api/buttons) and accepts clicks
 * (POST /api/buttons/<name>). A 200 response with body {"status":"ok"}
 * means the click was accepted; the button's observer callbacks run
 * afterwards on the event loop task, matching the /api/device/reset and
 * /api/device/restart contract.
 */
void add_button_handlers(std::shared_ptr<HTTPServer>& server);

}  // namespace sensesp

#endif  // SENSESP_NET_WEB_UI_BUTTON_HANDLER_H_
