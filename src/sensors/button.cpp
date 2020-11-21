#include "button.h"


Button::Button(uint8_t pin, String configPath, int repeat_start_interval, int repeat_interval, int pressedState) : 
    DigitalInput(pin, INPUT, CHANGE, configPath),
    repeat_start_interval{repeat_start_interval},
    repeat_interval{repeat_interval},
    pressedState{pressedState},
    last_press_sent{-1},
    pushed{false},
    repeating{false} {
    load_configuration();
}



void Button::enable() {
  debugD("Button enabled");
  // app.onInterrupt(pin, interrupt_type, [this]() {
  app.onRepeat(10, [this]() {
     int state = digitalRead(pin);
     bool nowPushed = (state == pressedState);
     if (nowPushed) {
         unsigned long interval = (last_press_sent != -1 ? millis() - last_press_sent : 0);
         bool start_repeat = (interval > repeat_start_interval);
         if (!pushed || start_repeat ||
            (repeating && interval > repeat_interval) ) {
            if (repeating) {
              debugD("Button PRESSED repeat");
            }
            else if (start_repeat) {
              debugD("Button PRESSED repeat starting");
              repeating = true;
            }
            else {
               debugD("Button state now PRESSED");
            }
            pushed = true;
            emit(pushed);
            last_press_sent = millis();
         }
     }
     else if (pushed) {
         debugD("Button state now UNPRESSED");
         pushed = false;
         emit(pushed);
         last_press_sent = -1;
         repeating = false;
     }
  });

}


void Button::get_configuration(JsonObject& root) {
  root["repeat_start_interval"] = repeat_start_interval;
  root["repeat_interval"] = repeat_interval;
}


static const char SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "repeat_start_interval": { "title": "Repeat initial button press after milliseconds [-1 to disable]", "type": "integer" },
        "repeat_interval": { "title": "Send repeat button press every milliseconds", "type": "integer" }
    }
  })";


String Button::get_config_schema() { return FPSTR(SCHEMA); }

bool Button::set_configuration(const JsonObject& config) {
  String expected[] = {"repeat_start_interval", "repeat_interval"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  repeat_start_interval = config["repeat_start_interval"];
  repeat_interval = config["repeat_interval"];
  return true;
}