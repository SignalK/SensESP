#include "frequency.h"

namespace sensesp {

// Frequency

Frequency::Frequency(float multiplier, const String& config_path)
    : Transform<int, float>(config_path),
      multiplier_{multiplier},
      last_update_(millis()) {
  load();
}

void Frequency::set(const int& input) {
  unsigned long const cur_millis = millis();
  if (first_call_) {
    first_call_ = false;
    last_update_ = cur_millis;
    return;
  }
  unsigned long const elapsed_millis = cur_millis - last_update_;
  last_update_ = cur_millis;
  this->emit(multiplier_ * input / (elapsed_millis / 1000.));
}

bool Frequency::to_json(JsonObject& root) {
  root["multiplier"] = multiplier_;
  return true;
}

bool Frequency::from_json(const JsonObject& config) {
  String const expected[] = {"multiplier"};
  for (auto str : expected) {
    if (!config[str].is<JsonVariant>()) {
      return false;
    }
  }
  multiplier_ = config["multiplier"];
  return true;
}

const String ConfigSchema(const Frequency& obj) {
  return R"###({"type":"object","properties":{"multiplier":{"title":"Multiplier","type":"number"}}})###";
}

}  // namespace sensesp
