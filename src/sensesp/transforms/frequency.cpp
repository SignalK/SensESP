#include "frequency.h"

namespace sensesp {

// Frequency

Frequency::Frequency(float multiplier, String config_path)
    : Transform<int, float>(config_path), multiplier_{multiplier} {
  load_configuration();

  last_update_ = millis();
}

void Frequency::set(const int& input) {
  unsigned long cur_millis = millis();
  unsigned long elapsed_millis = cur_millis - last_update_;
  last_update_ = cur_millis;
  this->emit(multiplier_ * input / (elapsed_millis / 1000.));
}

void Frequency::get_configuration(JsonObject& root) {
  root["multiplier"] = multiplier_;
}

static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "multiplier": { "title": "Multiplier", "type": "number" }
    }
  })###";

String Frequency::get_config_schema() { return FPSTR(SCHEMA); }

bool Frequency::set_configuration(const JsonObject& config) {
  String expected[] = {"multiplier"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  multiplier_ = config["multiplier"];
  return true;
}

}  // namespace sensesp
