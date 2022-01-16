#include "digital_input.h"

#include <elapsedMillis.h>
#include <FunctionalInterrupt.h>

#include "sensesp.h"

namespace sensesp {

void DigitalInputState::get_configuration(JsonObject& root) {
  root["read_delay"] = read_delay_;
}

static const char SCHEMA2[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "read_delay": { "title": "Read delay", "type": "number", "description": "The time, in milliseconds, between each read of the input" }
    }
  })###";

String DigitalInputState::get_config_schema() { return FPSTR(SCHEMA2); }

bool DigitalInputState::set_configuration(const JsonObject& config) {
  String expected[] = {"read_delay"};
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

static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "read_delay": { "title": "Read delay", "type": "number", "description": "The time, in milliseconds, between each read of the input" }
    }
  })###";

String DigitalInputCounter::get_config_schema() { return FPSTR(SCHEMA); }

bool DigitalInputCounter::set_configuration(const JsonObject& config) {
  String expected[] = {"read_delay"};
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

static const char DEBOUNCE_SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "read_delay": { "title": "Read delay", "type": "number", "description": "The time, in milliseconds, between each read of the input" },
        "ignore_interval": { "title": "Ignore interval", "type": "number", "description": "The time, in milliseconds, to ignore events after a recorded event" }
    }
  })###";

String DigitalInputDebounceCounter::get_config_schema() {
  return FPSTR(DEBOUNCE_SCHEMA);
}

bool DigitalInputDebounceCounter::set_configuration(const JsonObject& config) {
  String expected[] = {"read_delay", "ignore_interval"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      debugE(
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

void DigitalInputChange::start() {
  ReactESP::app->onInterrupt(pin_, interrupt_type_, [this]() {
    output = (bool)digitalRead(pin_);
    triggered_ = true;
  });

  ReactESP::app->onTick([this]() {
    if (triggered_ && (output != last_output_)) {
      noInterrupts();
      triggered_ = false;
      last_output_ = output;
      interrupts();
      notify();
    }
  });
}

}  // namespace sensesp
