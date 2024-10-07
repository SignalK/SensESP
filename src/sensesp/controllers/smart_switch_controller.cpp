#include "smart_switch_controller.h"

#include "sensesp/transforms/truth_text.h"
#include "sensesp_base_app.h"

namespace sensesp {

SmartSwitchController::SmartSwitchController(bool auto_initialize,
                                             String config_path,
                                             const char* sk_sync_paths[])
    : BooleanTransform(config_path), auto_initialize_{auto_initialize} {
  if (sk_sync_paths != NULL) {
    sync_paths_.clear();
    int i = 0;
    while (strlen(sk_sync_paths[i]) > 0) {
      SyncPath path(sk_sync_paths[i]);
      sync_paths_.insert(path);
      i++;
    }  // while
  }

  load();

  // Emit the initial state once the event loop starts
  if (auto_initialize_) {
    event_loop()->onDelay(0, [this]() { this->emit(is_on_); });
  }
}

void SmartSwitchController::set(const bool& new_value) {
  is_on_ = new_value;
  this->emit(is_on_);
}

void SmartSwitchController::set(const ClickTypes& new_value) {
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
  is_on_ = !is_on_;
  this->emit(is_on_);

  if (new_value == ClickTypes::DoubleClick) {
    // Sync any specified sync paths...
    for (auto& path : sync_paths_) {
      ESP_LOGD(__FILENAME__, "Sync status to %s", path.sk_sync_path_.c_str());
      path.put_request_->set(is_on_);
    }
  }
}

void SmartSwitchController::set(const String& new_value) {
  if (TextToTruth::is_valid_true(new_value)) {
    is_on_ = true;
  } else if (TextToTruth::is_valid_false(new_value)) {
    is_on_ = false;
  } else {
    // All other values simply toggle...
    is_on_ = !is_on_;
  }
  this->emit(is_on_);
}

bool SmartSwitchController::to_json(JsonObject& root) {
  JsonArray jPaths = root["sync_paths"].to<JsonArray>();
  for (auto& path : sync_paths_) {
    jPaths.add(path.sk_sync_path_);
  }
  return true;
}

bool SmartSwitchController::from_json(const JsonObject& config) {
  JsonArray arr = config["sync_paths"];
  if (arr.size() > 0) {
    sync_paths_.clear();
    for (String sk_path : arr) {
      SyncPath path(sk_path);
      sync_paths_.insert(path);
    }
  }

  return true;
}

SmartSwitchController::SyncPath::SyncPath() {}

SmartSwitchController::SyncPath::SyncPath(String sk_sync_path)
    : sk_sync_path_{sk_sync_path} {
  ESP_LOGD(__FILENAME__, "DoubleClick will also sync %s", sk_sync_path.c_str());
  this->put_request_ = new BoolSKPutRequest(sk_sync_path, "", false);
}

}  // namespace sensesp
