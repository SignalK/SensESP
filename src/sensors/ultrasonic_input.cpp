#include "ultrasonic_input.h"

#include "Arduino.h"

#include "sensesp.h"



UltrasonicInput::UltrasonicInput(uint read_delay, String config_path) 
                 : NumericSensor(config_path), read_delay{read_delay} {
  className = "UltrasonicInput";
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(INPUT_PIN, INPUT_PULLUP);
  load_configuration();
}

void ICACHE_RAM_ATTR  UltrasonicInput::changeISR(){
  if (state == 0) {
    if (digitalRead(INPUT_PIN) == HIGH){
      state = 1;
      startTime = micros();
    } else {
      return ;
    }
  } else { // State is 1
    if (digitalRead(INPUT_PIN) == LOW){
      pulseWidth = (micros() - startTime);
      return ;
      state = 0;
    } else {
      return ;
    }
  }
}

void UltrasonicInput::update() {
  output = ultrasonicRead();
  this->notify();
}

int UltrasonicInput::ultrasonicRead(){
    digitalWrite(TRIGGER_PIN, HIGH);
    long lastTime = micros();
      while (micros() - lastTime < 100)
      {
        yield();
      }
      digitalWrite(TRIGGER_PIN, LOW);
      return pulseWidth;    // This is actually the pulse width from the previous trigger
}

void UltrasonicInput::enable() {
  app.onRepeat(read_delay, [this](){ this->update(); });
}

JsonObject& UltrasonicInput::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["read_delay"] = read_delay;
  root["value"] = output;
  return root;
  };

  static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "read_delay": { "title": "Read delay", "type": "number", "description": "Number of milliseconds between each ultrasonicRead(INPUT_PIN)" },
        "value": { "title": "Last value", "type" : "number", "readOnly": true }
    }
  })###";

  String UltrasonicInput::get_config_schema() {
  return FPSTR(SCHEMA);
}

bool UltrasonicInput::set_configuration(const JsonObject& config) {
  String expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  read_delay = config["read_delay"];
  return true;
}
