#include "smart_switch_controller.h"

#include "sensesp/transforms/truth_text.h"

namespace sensesp {

SmartSwitchController::SmartSwitchController(bool auto_initialize,
                                             String config_path,
                                             const char* sk_sync_paths[])
    : BooleanTransform(config_path), auto_initialize_{auto_initialize} {
  if (sk_sync_paths != NULL) {
    sync_paths.clear();
    int i = 0;
    while (strlen(sk_sync_paths[i]) > 0) {
      SyncPath path(sk_sync_paths[i]);
      sync_paths.insert(path);
      i++;
    }  // while
  }

  load_configuration();

  // Emit the initial state once the event loop starts
  if (auto_initialize_) {
    ReactESP::app->onDelay(0, [this]() { this->emit(is_on); });
  }
}

void SmartSwitchController::set_input(bool new_value, uint8_t input_channel) {
  is_on = new_value;
  this->emit(is_on);
}

void SmartSwitchController::set_input(ClickTypes new_value,
                                      uint8_t input_channel) {
  if (!ClickType::is_click(new_value)) {
    // Ignore button presses (we only want interpreted clicks)
    return;
  }

  if (new_value == ClickTypes::UltraLongSingleClick) {
    // Long clicks reboot the system...
    ESP.restart();
    return;
  }

  // All other click types toggle the current state...
  is_on = !is_on;
  this->emit(is_on);

  if (new_value == ClickTypes::DoubleClick) {
    // Sync any specified sync paths...
    for (auto& path : sync_paths) {
      debugD("Sync status to %s", path.sk_sync_path.c_str());
      path.put_request->set_input(is_on);
    }
  }
}

void SmartSwitchController::set_input(String new_value, uint8_t input_channel) {
  if (TextToTruth::is_valid_true(new_value)) {
    is_on = true;
  } else if (TextToTruth::is_valid_false(new_value)) {
    is_on = false;
  } else {
    // All other values simply toggle...
    is_on = !is_on;
  }
  this->emit(is_on);
}

void SmartSwitchController::get_configuration(JsonObject& root) {
  JsonArray jPaths = root["sync_paths"].to<JsonArray>();
  for (auto& path : sync_paths) {
    jPaths.add(path.sk_sync_path);
  }
}

static const char SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "sync_paths": { "title": "Sync on double click",
                        "type": "array",
                        "items": { "type": "string"}
        }
    }
  })";

String SmartSwitchController::get_config_schema() { return FPSTR(SCHEMA); }

bool SmartSwitchController::set_configuration(const JsonObject& config) {
  JsonArray arr = config["sync_paths"];
  if (arr.size() > 0) {
    sync_paths.clear();
    for (String sk_path : arr) {
      SyncPath path(sk_path);
      sync_paths.insert(path);
    }
  }

  return true;
}

SmartSwitchController::SyncPath::SyncPath() {}

SmartSwitchController::SyncPath::SyncPath(String sk_sync_path)
    : sk_sync_path{sk_sync_path} {
  debugD("DoubleClick will also sync %s", sk_sync_path.c_str());
  this->put_request = new BoolSKPutRequest(sk_sync_path, "", false);
}

}  // namespace sensesp
