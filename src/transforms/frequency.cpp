#include "frequency.h"

// Frequency

Frequency::Frequency(float multiplier, String config_path)
    : Transform<int, float>(config_path), multiplier_{multiplier} {
  load_configuration();
}

void Frequency::enable() { last_update_ = millis(); }

void Frequency::set_input(int input, uint8_t inputChannel) {
  unsigned long cur_millis = millis();
  unsigned long elapsed_millis = cur_millis - last_update_;
  last_update_ = cur_millis;
  this->emit(multiplier_ * input / (elapsed_millis / 1000.));
}

void Frequency::get_configuration(JsonObject& root) {
  root["multiplier"] = multiplier_;
  root["value"] = output;
}

static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "multiplier": { "title": "Multiplier", "type": "number" },
        "value": { "title": "Last value", "type" : "number", "readOnly": true }
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
