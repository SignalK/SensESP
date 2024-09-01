#include "frequency.h"

namespace sensesp {

// Frequency

Frequency::Frequency(float multiplier, const String& config_path)
    : Transform<int, float>(config_path),
      multiplier_{multiplier},
      last_update_(millis()) {
  load_configuration();
}

void Frequency::set(const int& input) {
  uint64_t const cur_millis = millis();
  uint64_t const elapsed_millis = cur_millis - last_update_;
  last_update_ = cur_millis;
  this->emit(multiplier_ * input / (elapsed_millis / 1000.));
}

void Frequency::get_configuration(JsonObject& root) {
  root["multiplier"] = multiplier_;
}

static const char kSchema[] = R"###({
    "type": "object",
    "properties": {
        "multiplier": { "title": "Multiplier", "type": "number" }
    }
  })###";

String Frequency::get_config_schema() { return (kSchema); }

bool Frequency::set_configuration(const JsonObject& config) {
  String const expected[] = {"multiplier"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  multiplier_ = config["multiplier"];
  return true;
}

}  // namespace sensesp
