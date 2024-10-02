#include "sensesp.h"

#include "digital_input.h"

#include <FunctionalInterrupt.h>
#include <elapsedMillis.h>

namespace sensesp {

bool DigitalInputState::to_json(JsonObject& root) {
  root["read_delay"] = read_delay_;
  return true;
}

bool DigitalInputState::from_json(const JsonObject& config) {
  String const expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config[str].is<JsonVariant>()) {
      return false;
    }
  }
  read_delay_ = config["read_delay"];
  return true;
}

bool DigitalInputCounter::to_json(JsonObject& root) {
  root["read_delay"] = read_delay_;
  return true;
}

bool DigitalInputCounter::from_json(const JsonObject& config) {
  String const expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config[str].is<JsonVariant>()) {
      return false;
    }
  }
  read_delay_ = config["read_delay"];
  return true;
}

void DigitalInputDebounceCounter::handleInterrupt() {
  if (since_last_event_ > ignore_interval_ms_) {
    counter_++;
    since_last_event_ = 0;
  }
}

bool DigitalInputDebounceCounter::to_json(JsonObject& root) {
  root["read_delay"] = read_delay_;
  root["ignore_interval"] = ignore_interval_ms_;
  return true;
}

bool DigitalInputDebounceCounter::from_json(const JsonObject& config) {
  String const expected[] = {"read_delay", "ignore_interval"};
  for (auto str : expected) {
    if (!config[str].is<JsonVariant>()) {
      ESP_LOGE(
          __FILENAME__,
          "Cannot set DigitalInputDebounceConfiguration configuration: missing "
          "json field %s",
          str.c_str());
      return false;
    }
  }

  read_delay_ = config["read_delay"];
  ignore_interval_ms_ = config["ignore_interval"];
  return true;
}

const String ConfigSchema(DigitalInputState& obj) {
  return R"###({
    "type": "object",
    "properties": {
        "read_delay": { "title": "Read delay", "type": "number", "description": "The time, in milliseconds, between each read of the input" }
    }
  })###";
}

bool ConfigRequiresRestart(const DigitalInputState& obj) {
  return true;
}

const String ConfigSchema(const DigitalInputCounter& obj) {
  return R"###({
    "type": "object",
    "properties": {
        "read_delay": { "title": "Read delay", "type": "number", "description": "The time, in milliseconds, between each read of the input" }
    }
  })###";
}

bool ConfigRequiresRestart(const DigitalInputCounter& obj) {
  return true;
}

const String ConfigSchema(const DigitalInputDebounceCounter& obj) {
  return R"###({
    "type": "object",
    "properties": {
        "read_delay": { "title": "Read delay", "type": "number", "description": "The time, in milliseconds, between each read of the input" },
        "ignore_interval": { "title": "Ignore interval", "type": "number", "description": "The time, in milliseconds, to ignore events after a recorded event" }
    }
  })###";
}

bool ConfigRequiresRestart(const DigitalInputDebounceCounter& obj) {
  return true;
}

}  // namespace sensesp
