#include "analog_input.h"

#include "Arduino.h"

#include "sensesp.h"

AnalogInput::AnalogInput(uint8_t pin, uint read_delay, String config_path) 
                 : NumericSensor(config_path), pin{pin}, read_delay{read_delay} {
  pinMode(pin, INPUT);
  className = "AnalogInput";
  load_configuration();
}

void AnalogInput::update() {
  output = analogRead(pin);
  this->notify();
}

void AnalogInput::enable() {
  app.onRepeat(read_delay, [this](){ this->update(); });
}

JsonObject& AnalogInput::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["read_delay"] = read_delay;
  root["value"] = output;
  return root;
  };

  static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "read_delay": { "title": "Read delay", "type": "number", "description": "Number of milliseconds between each analogRead(A0)" },
        "value": { "title": "Last value", "type" : "number", "readOnly": true }
    }
  })###";

  String AnalogInput::get_config_schema() {
  return FPSTR(SCHEMA);
}

bool AnalogInput::set_configuration(const JsonObject& config) {
  String expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  read_delay = config["read_delay"];
  return true;
}
