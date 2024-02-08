#ifndef SENSESP_NET_WEB_BASE_COMMAND_HANDLER_H_
#define SENSESP_NET_WEB_BASE_COMMAND_HANDLER_H_

#include <esp_http_server.h>

#include <vector>

#include "ArduinoJson.h"
#include "sensesp/net/http_server.h"
#include "sensesp/net/web/autogen/web_ui_files.h"
#include "sensesp/net/web/static_file_handler.h"
#include "sensesp/system/configurable.h"
#include "sensesp/ui/ui_output.h"
#include "sensesp_base_app.h"

namespace sensesp {

class RouteDefinition {
 public:
  RouteDefinition(String name, String path, String component_name)
      : name_(name), path_(path), component_name_(component_name){};

  String get_name() { return name_; }
  String get_path() { return path_; }
  String get_component_name() { return component_name_; }

  JsonDocument as_json() {
    JsonDocument doc;
    doc["name"] = name_;
    doc["path"] = path_;
    doc["componentName"] = component_name_;

    return doc;
  }

 protected:
  String name_;
  String path_;
  String component_name_;
};

void add_base_app_http_command_handlers(HTTPServer* server);

}  // namespace sensesp

#endif  // SENSESP_NET_HTTP_COMMAND_HANDLER_H_
