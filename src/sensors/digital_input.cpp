#include "digital_input.h"

#include <elapsedMillis.h>
#include <FunctionalInterrupt.h>

#include "sensesp.h"

DigitalInput::DigitalInput(uint8_t pin, int pin_mode, int interrupt_type,
                           String config_path)
    : Sensor(config_path), pin_{pin}, interrupt_type_{interrupt_type} {
  pinMode(pin, pin_mode);
}

DigitalInputState::DigitalInputState(uint8_t pin, int pin_mode,
                                     int interrupt_type, int read_delay,
                                     String config_path)
    : DigitalInput{pin, pin_mode, interrupt_type, config_path},
      IntegerProducer(),
      read_delay_{read_delay},
      triggered_{false} {
  load_configuration();      
}

void DigitalInputState::enable() {
  app.onRepeat(read_delay_, [this]() {
    emit(digitalRead(pin_));
  });
}

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

DigitalInputCounter::DigitalInputCounter(uint8_t pin, int pin_mode,
                                         int interrupt_type, uint read_delay,
                                         String config_path)
    : DigitalInputCounter(pin, pin_mode, interrupt_type, read_delay,
        config_path, [this]() {this->counter_++;}) {}

DigitalInputCounter::DigitalInputCounter(uint8_t pin, int pin_mode,
                                         int interrupt_type, uint read_delay,
                                         String config_path,
                                         std::function<void()> interrupt_handler)
    : DigitalInput{pin, pin_mode, interrupt_type, config_path},
      IntegerProducer(),
      interrupt_handler_{interrupt_handler},
      read_delay_{read_delay} {
  load_configuration();
}

void DigitalInputCounter::enable() {
  app.onInterrupt(pin_, interrupt_type_, interrupt_handler_);

  app.onRepeat(read_delay_, [this]() {
    noInterrupts();
    output = counter_;
    counter_ = 0;
    interrupts();
    notify();
  });
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

// DEBOUNCE
DigitalInputDebounceCounter::DigitalInputDebounceCounter(
    uint8_t pin, int pin_mode, int interrupt_type, unsigned int read_delay_ms,
    unsigned int ignore_interval_ms, String config_path)
    : DigitalInputCounter(pin, pin_mode, interrupt_type, read_delay_ms,
        config_path, [this]() {this->handleInterrupt();}),
      ignore_interval_ms_{ignore_interval_ms} {
}

void DigitalInputDebounceCounter::handleInterrupt() {
  if (since_last_event_ > ignore_interval_ms_) {
    this->counter_++;
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

DigitalInputChange::DigitalInputChange(uint8_t pin, int pin_mode,
                                       int interrupt_type, uint read_delay,
                                       String config_path)
    : DigitalInput(pin, pin_mode, interrupt_type, config_path),
      IntegerProducer(),
      read_delay_{read_delay},
      triggered_{false},
      last_output_{0},
      value_sent_{false} {
    load_configuration();    
    }

void DigitalInputChange::enable() {
  app.onInterrupt(pin_, interrupt_type_,
    [this](){
      output = digitalRead(pin_);
      triggered_ = true;
    });
  
  
  app.onRepeat(read_delay_, [this](){
      if (triggered_ && (output != last_output_ || value_sent_ == false)) {
        noInterrupts();
        triggered_ = false;
        last_output_ = output;
        interrupts();
        value_sent_ = true;
        notify();
      }
    }
  );
}

void DigitalInputChange::get_configuration(JsonObject& root) {
  root["read_delay"] = read_delay_;
}

static const char SCHEMA3[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "read_delay": { "title": "Read delay", "type": "number", "description": "The time, in milliseconds, between each read of the input" }
    }
  })###";

String DigitalInputChange::get_config_schema() { return FPSTR(SCHEMA3); }

bool DigitalInputChange::set_configuration(const JsonObject& config) {
  String expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  read_delay_ = config["read_delay"];
  return true;
}
