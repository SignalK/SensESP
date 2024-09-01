#include "sensesp.h"

#include "digital_input.h"

#include <FunctionalInterrupt.h>
#include <elapsedMillis.h>

namespace sensesp {

void DigitalInputState::get_configuration(JsonObject& root) {
  root["read_delay"] = read_delay_;
}

static const char kSchema2[] = R"###({
    "type": "object",
    "properties": {
        "read_delay": { "title": "Read delay", "type": "number", "description": "The time, in milliseconds, between each read of the input" }
    }
  })###";

String DigitalInputState::get_config_schema() { return kSchema2; }

bool DigitalInputState::set_configuration(const JsonObject& config) {
  String const expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  read_delay_ = config["read_delay"];
  return true;
}

void DigitalInputCounter::get_configuration(JsonObject& root) {
  root["read_delay"] = read_delay_;
}

static const char kSchema[] = R"###({
    "type": "object",
    "properties": {
        "read_delay": { "title": "Read delay", "type": "number", "description": "The time, in milliseconds, between each read of the input" }
    }
  })###";

String DigitalInputCounter::get_config_schema() { return kSchema; }

bool DigitalInputCounter::set_configuration(const JsonObject& config) {
  String const expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
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

void DigitalInputDebounceCounter::get_configuration(JsonObject& root) {
  root["read_delay"] = read_delay_;
  root["ignore_interval"] = ignore_interval_ms_;
}

static const char kDebounceSchema[] = R"###({
    "type": "object",
    "properties": {
        "read_delay": { "title": "Read delay", "type": "number", "description": "The time, in milliseconds, between each read of the input" },
        "ignore_interval": { "title": "Ignore interval", "type": "number", "description": "The time, in milliseconds, to ignore events after a recorded event" }
    }
  })###";

String DigitalInputDebounceCounter::get_config_schema() {
  return kDebounceSchema;
}

bool DigitalInputDebounceCounter::set_configuration(const JsonObject& config) {
  String const expected[] = {"read_delay", "ignore_interval"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
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

}  // namespace sensesp
