#include "ultrasonic_input.h"
#include "Arduino.h"
#include "sensesp.h"

UltrasonicSens::UltrasonicSens(int8_t trig_pin, int8_t input_pin,  uint read_delay, String config_path) :
    NumericSensor(config_path), triggerPin{trig_pin}, inputPin{input_pin}, read_delay{read_delay} {
  className = "UltrasonicSens";
  pinMode(trig_pin, OUTPUT);
  pinMode(input_pin, INPUT_PULLUP);
  load_configuration();
}

void UltrasonicSens::enable() {
  app.onRepeat(read_delay, [this]() { 
    digitalWrite(triggerPin, HIGH);
  long lastTime = micros();
  while (micros() - lastTime < 100) {
    yield();
  }
  digitalWrite(triggerPin, LOW);
  output = pulseIn(inputPin, HIGH);
  this->notify();
  });
}

JsonObject& UltrasonicSens::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["read_delay"] = read_delay;
  root["value"] = output;
  return root;
};

static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "read_delay": { "title": "Read delay", "type": "number", "description": "Number of milliseconds between each thermocouple read " },
        "value": { "title": "Last value", "type" : "number", "readOnly": true }
    }
  })###";

String UltrasonicSens::get_config_schema() { return FPSTR(SCHEMA); }

bool UltrasonicSens::set_configuration(const JsonObject& config) {
  String expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  read_delay = config["read_delay"];
  return true;
}
