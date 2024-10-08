#include "analogvoltage.h"

namespace sensesp {

AnalogVoltage::AnalogVoltage(float max_voltage, float multiplier, float offset,
                             const String& config_path)
    : FloatTransform(config_path),
      max_voltage_{max_voltage},
      multiplier_{multiplier},
      offset_{offset} {
  load();
}

void AnalogVoltage::set(const float& input) {
  this->emit(((input * (max_voltage_ / MAX_ANALOG_OUTPUT)) * multiplier_) +
             offset_);
}

bool AnalogVoltage::to_json(JsonObject& root) {
  root["max_voltage"] = max_voltage_;
  root["multiplier"] = multiplier_;
  root["offset"] = offset_;
  return true;
}

bool AnalogVoltage::from_json(const JsonObject& config) {
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

const String ConfigSchema(const AnalogVoltage& obj) {
  return R"###({"type":"object","properties":{"max_voltage":{"title":"Max voltage","type":"number","description":"The maximum voltage allowed into your ESP's Analog Input pin"},"multiplier":{"title":"Multiplier","type":"number","description":"Output will be multiplied by this before sending to SK"},"offset":{"title":"Offset","type":"number","description":"This will be added to output before sending to SK"}}})###";
}

}  // namespace sensesp
