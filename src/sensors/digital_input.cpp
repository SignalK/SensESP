#include "digital_input.h"

#include <FunctionalInterrupt.h>

#include "sensesp.h"

DigitalInput::DigitalInput(
    uint8_t pin, int pin_mode, int interrupt_type,
    String config_path)
    : Sensor(config_path), pin{pin}, interrupt_type{interrupt_type} {
  className = "DigitalInput";
  pinMode(pin, pin_mode);
}

DigitalInputValue::DigitalInputValue(
    uint8_t pin, int pin_mode, int interrupt_type, int read_delay,
    String config_path) :
      DigitalInput{pin, pin_mode, interrupt_type, config_path},
      IntegerProducer(), read_delay{read_delay} {
        className = "DigitalInputValue";
      }

void DigitalInputValue::enable() {
  app.onRepeat(read_delay, [this](){
    output = digitalRead(pin);
        notify();
  });
}

JsonObject& DigitalInputValue::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["read_delay"] = read_delay;
  root["value"] = output;
  return root;
  };

  static const char SCHEMA2[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "read_delay": { "title": "Read delay", "type": "number", "description": "The time, in milliseconds, between each read of the input" },
        "value": { "title": "Last value", "type" : "number", "readOnly": true }
    }
  })###";

  String DigitalInputValue::get_config_schema() {
  return FPSTR(SCHEMA2);
}

bool DigitalInputValue::set_configuration(const JsonObject& config) {
  String expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  read_delay = config["read_delay"];
  return true;
}


DigitalInputCounter::DigitalInputCounter(
    uint8_t pin, int pin_mode, int interrupt_type,
    uint read_delay,
    String config_path) :
      DigitalInput{pin, pin_mode, interrupt_type, config_path},
      IntegerProducer(),
      read_delay{read_delay} {
        className = "DigitalInputCounter";
        load_configuration();
      }

void DigitalInputCounter::enable() {
  app.onInterrupt(pin, interrupt_type,
                  [this](){
    this->counter++;
  });

  app.onRepeat(read_delay, [this](){
    noInterrupts();
    output = counter;
    counter = 0;
    interrupts();
    notify();
  });
  }

  JsonObject& DigitalInputCounter::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["read_delay"] = read_delay;
  root["value"] = output;
  return root;
  };

  static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "read_delay": { "title": "Read delay", "type": "number", "description": "The time, in milliseconds, between each read of the input" },
        "value": { "title": "Last value", "type" : "number", "readOnly": true }
    }
  })###";

  String DigitalInputCounter::get_config_schema() {
  return FPSTR(SCHEMA);
}

bool DigitalInputCounter::set_configuration(const JsonObject& config) {
  String expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  read_delay = config["read_delay"];
  return true;
}
