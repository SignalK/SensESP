#include "analogvoltage.h"

namespace sensesp {

AnalogVoltage::AnalogVoltage(float max_voltage, float multiplier, float offset,
                             const String& config_path)
    : FloatTransform(config_path),
      max_voltage_{max_voltage},
      multiplier_{multiplier},
      offset_{offset} {
  load_configuration();
}

void AnalogVoltage::set(const float& input) {
  this->emit(((input * (max_voltage_ / MAX_ANALOG_OUTPUT)) * multiplier_) +
             offset_);
}

void AnalogVoltage::get_configuration(JsonObject& root) {
  root["max_voltage"] = max_voltage_;
  root["multiplier"] = multiplier_;
  root["offset"] = offset_;
}

static const char kSchema[] = R"({
    "type": "object",
    "properties": {
        "max_voltage": { "title": "Max voltage", "type": "number", "description": "The maximum voltage allowed into your ESP's Analog Input pin" },
        "multiplier": { "title": "Mulitplier", "type": "number", "description": "Output will be multiplied by this before sending to SK" },
        "offset": { "title": "Offset", "type": "number", "description": "This will be added to output before sending to SK" }
    }
  })";

String AnalogVoltage::get_config_schema() { return kSchema; }

bool AnalogVoltage::set_configuration(const JsonObject& config) {
  const String expected[] = {"max_voltage", "multiplier", "offset"};
  for (auto str : expected) {
    if (!config[str].is<JsonVariant>()) {
      return false;
    }
  }
  max_voltage_ = config["max_voltage"];
  multiplier_ = config["multiplier"];
  offset_ = config["offset"];
  return true;
}

}  // namespace sensesp
