#include "press_repeater.h"

namespace sensesp {

PressRepeater::PressRepeater(const String& config_path, int integer_false,
                             int repeat_start_interval, int repeat_interval)
    : BooleanTransform(config_path),
      integer_false_{integer_false},
      repeat_start_interval_{repeat_start_interval},
      repeat_interval_{repeat_interval},
      pushed_{false},
      repeating_{false} {
  load();

  event_loop()->onRepeat(10, [this]() {
    if (pushed_) {
      // A press is currently in progress
      if (repeating_) {
        if (last_value_sent_ > static_cast<uint64_t>(repeat_interval_)) {
          ESP_LOGD(__FILENAME__, "Repeating press report");
          last_value_sent_ = 0;
          this->emit(true);
        }
      } else if (last_value_sent_ >
                 static_cast<uint64_t>(repeat_start_interval_)) {
        ESP_LOGD(__FILENAME__, "Starting press report repeat");
        repeating_ = true;
        last_value_sent_ = 0;
        this->emit(true);
      }
    }
  });
}

void PressRepeater::set(const bool& new_value) {
  if (new_value != pushed_) {
    pushed_ = new_value;

    if (!pushed_) {
      repeating_ = false;
    }

    last_value_sent_ = 0;
    this->emit(pushed_);
  }
}

bool PressRepeater::to_json(JsonObject& root) {
  root["repeat_start_interval"] = repeat_start_interval_;
  root["repeat_interval"] = repeat_interval_;
  return true;
}

bool PressRepeater::from_json(const JsonObject& config) {
  String const expected[] = {"repeat_start_interval", "repeat_interval"};
  for (auto str : expected) {
    if (!config[str].is<JsonVariant>()) {
      return false;
    }
  }
  repeat_start_interval_ = config["repeat_start_interval"];
  repeat_interval_ = config["repeat_interval"];
  return true;
}

const String ConfigSchema(const PressRepeater& obj) {
  return R"###({"type":"object","properties":{"repeat_start_interval":{"title":"Start repeating after (ms)","type":"integer"},"repeat_interval":{"title":"Repeat report interval (ms)","type":"integer"}}})###";
}

}  // namespace sensesp
