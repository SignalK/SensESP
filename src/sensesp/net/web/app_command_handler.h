#ifndef SENSESP_NET_HTTP_APP_COMMAND_HANDLER_H_
#define SENSESP_NET_HTTP_APP_COMMAND_HANDLER_H_

#include <memory>
#include <vector>

#include "ArduinoJson.h"
#include "sensesp/net/http_server.h"
#include "sensesp/net/wifi_provisioner.h"

namespace sensesp {

/// Register the transport-agnostic Signal K TOFU reset handler.
/// Safe to call regardless of which network provisioner is active.
void add_tofu_reset_handler(std::shared_ptr<HTTPServer>& server);

/// Register the WiFi-only command handlers (network scan + TOFU reset).
/// Should only be called when WiFi is the active network provisioner.
/// `wifi_provisioner` is taken by value so callers can pass the result
/// of SensESPApp::get_wifi_provisioner() (which returns a temporary).
void add_app_http_command_handlers(
    std::shared_ptr<HTTPServer>& server,
    std::shared_ptr<WiFiProvisioner> wifi_provisioner);

}  // namespace sensesp

#endif
