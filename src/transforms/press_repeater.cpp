#include "transforms/press_repeater.h"


PressRepeater::PressRepeater(String config_path, int integer_false, int repeat_start_interval, int repeat_interval) : 
    BooleanTransform(config_path),
    integer_false{integer_false},
    repeat_start_interval{repeat_start_interval},
    repeat_interval{repeat_interval},
    pushed{false},
    repeating{false} {
    load_configuration();
}


void PressRepeater::enable() {

  app.onRepeat(10, [this]() {

     if (pushed) {
         // A press is currently in progress
         if (repeating) {
            if (last_value_sent > (unsigned long)repeat_interval) {
                debugD("Repeating press report");
                last_value_sent = 0;
                this->emit(true);
            }
         }
         else if (last_value_sent > (unsigned long)repeat_start_interval) {
            debugD("Starting press report repeat");
            repeating = true;
            last_value_sent = 0;
            this->emit(true);
         }
     }
  });

}


void PressRepeater::set_input(int new_value, uint8_t input_channel) {
   this->set_input(new_value != integer_false, input_channel);
}


void PressRepeater::set_input(bool new_value, uint8_t input_channel) {
   if (new_value != pushed) {
      pushed = new_value;

      if (!pushed) {
         repeating = false;
      }

      last_value_sent = 0;
      this->emit(pushed);
   }
}


void PressRepeater::get_configuration(JsonObject& root) {
  root["repeat_start_interval"] = repeat_start_interval;
  root["repeat_interval"] = repeat_interval;
}


static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "repeat_start_interval": { "title": "Start repeating after (ms)", "type": "integer" },
        "repeat_interval": { "title": "Repeat report interval (ms)", "type": "integer" }
    }
})###";


String PressRepeater::get_config_schema() { return FPSTR(SCHEMA); }

bool PressRepeater::set_configuration(const JsonObject& config) {
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