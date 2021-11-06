#include "transforms/press_repeater.h"

namespace sensesp {

PressRepeater::PressRepeater(String config_path, int integer_false,
                             int repeat_start_interval, int repeat_interval)
    : BooleanTransform(config_path),
      integer_false_{integer_false},
      repeat_start_interval_{repeat_start_interval},
      repeat_interval_{repeat_interval},
      pushed_{false},
      repeating_{false} {
  load_configuration();
}

void PressRepeater::start() {
  app.onRepeat(10, [this]() {
    if (pushed_) {
      // A press is currently in progress
      if (repeating_) {
        if (last_value_sent_ > (unsigned long)repeat_interval_) {
          debugD("Repeating press report");
          last_value_sent_ = 0;
          this->emit(true);
        }
      } else if (last_value_sent_ > (unsigned long)repeat_start_interval_) {
        debugD("Starting press report repeat");
        repeating_ = true;
        last_value_sent_ = 0;
        this->emit(true);
      }
    }
  });
}

void PressRepeater::set_input(int new_value, uint8_t input_channel) {
  this->set_input(new_value != integer_false_, input_channel);
}

void PressRepeater::set_input(bool new_value, uint8_t input_channel) {
  if (new_value != pushed_) {
    pushed_ = new_value;

    if (!pushed_) {
      repeating_ = false;
    }

    last_value_sent_ = 0;
    this->emit(pushed_);
  }
}

void PressRepeater::get_configuration(JsonObject& root) {
  root["repeat_start_interval"] = repeat_start_interval_;
  root["repeat_interval"] = repeat_interval_;
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
  repeat_start_interval_ = config["repeat_start_interval"];
  repeat_interval_ = config["repeat_interval"];
  return true;
}

}  // namespace sensesp
