#ifndef _enable_H_
#define _enable_H_

#include "transforms/transform.h"

namespace sensesp {

static const char ENABLE_TRANSFORM_SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
      "enabled": { "type": "boolean", "title": "Enable", "description": "Enable or disable the transform output" }
    }
  })";

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
    this->load_configuration();
  }
  virtual void set_input(T input, uint8_t input_channel = 0) override {
    if (enabled_) {
      this->emit(input);
    }
  }
  virtual void get_configuration(JsonObject& doc) override {
    doc["enabled"] = enabled_;
  }
  virtual bool set_configuration(const JsonObject& config) override {
    if (config.containsKey("enabled")) {
      enabled_ = config["enabled"];
    } else {
      return false;
    }
    return true;
  }
  virtual String get_config_schema() override {
    return FPSTR(ENABLE_TRANSFORM_SCHEMA);
  };

 private:
  bool enabled_;
};

}  // namespace sensesp

#endif
