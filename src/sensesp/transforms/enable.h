#ifndef SENSESP_SRC_SENSESP_TRANSFORMS_ENABLE_H_
#define SENSESP_SRC_SENSESP_TRANSFORMS_ENABLE_H_

#include "sensesp/ui/config_item.h"
#include "sensesp/transforms/transform.h"

namespace sensesp {

/**
 * @brief On/off switch for signals: input is emitted as-is if the enable flag
 * is set in the web UI.
 *
 * Enable transform can be used to enable or disable certain transforms.
 * When set to false, the transform will not emit any output.
 *
 * @tparam T Input and output type.
 */
template <typename T>
class Enable : public Transform<T, T> {
 public:
  Enable(bool enabled = true, String config_path = "")
      : Transform<T, T>(config_path) {
    this->enabled_ = enabled;
    this->load();
  }
  virtual void set(const T& input) override {
    if (enabled_) {
      this->emit(input);
    }
  }
  virtual bool to_json(JsonObject& doc) override {
    doc["enabled"] = enabled_;
    return true;
  }
  virtual bool from_json(const JsonObject& config) override {
    if (config["enabled"].is<bool>()) {
      enabled_ = config["enabled"];
    } else {
      return false;
    }
    return true;
  }

 private:
  bool enabled_;
};

template <typename U>
const String ConfigSchema(const Enable<U>& obj) {
  return R"({
    "type": "object",
    "properties": {
      "enabled": { "type": "boolean", "title": "Enable", "description": "Enable or disable the transform output" }
    }
  })";
}

}  // namespace sensesp

#endif
