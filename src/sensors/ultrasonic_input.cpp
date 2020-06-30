#include "ultrasonic_input.h"
#include "Arduino.h"
#include "sensesp.h"

uint triggerPin;
uint inputPin;

UltrasonicSens::UltrasonicSens(int8_t trig_pin, int8_t input_pin, String config_path) :
    Sensor(config_path) {
  className = "UltrasonicSens";
  pinMode(trig_pin, OUTPUT);
  pinMode(input_pin, INPUT_PULLUP);
  triggerPin = trig_pin;
  inputPin = input_pin;
  load_configuration();
}

UltrasonicSensValue::UltrasonicSensValue(UltrasonicSens* pUltrasonicSens, uint read_delay, String config_path) :
  NumericSensor(config_path), pUltrasonicSens{pUltrasonicSens}, read_delay{read_delay} {
    className = "UltrasonicSensValue";
    load_configuration();
  }

void UltrasonicSensValue::enable() {
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

JsonObject& UltrasonicSensValue::get_configuration(JsonBuffer& buf) {
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

String UltrasonicSensValue::get_config_schema() { return FPSTR(SCHEMA); }

bool UltrasonicSensValue::set_configuration(const JsonObject& config) {
  String expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  read_delay = config["read_delay"];
  return true;
}
