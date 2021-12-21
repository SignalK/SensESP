#include "angle_correction.h"

namespace sensesp {

#define M_PI 3.14159265358979323846

AngleCorrection::AngleCorrection(float offset, float min_angle,
                                 String config_path)
    : FloatTransform(config_path), offset_{offset}, min_angle_{min_angle} {
  load_configuration();
}

void AngleCorrection::set_input(float input, uint8_t inputChannel) {
  // first the correction
  float x = input + offset_;

  // then wrap around the values
  x = fmod(x - min_angle_, 2 * M_PI);
  if (x < 0) {
    x += 2 * M_PI;
  }
  this->emit(x + min_angle_);
}

void AngleCorrection::get_configuration(JsonObject& root) {
  root["offset"] = offset_;
  root["min_angle"] = min_angle_;
}

static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "offset": { "title": "Constant offset", "description": "Value to be added, in radians", "type": "number" },
        "min_angle": { "title": "Minimum angle value", "description": "If you have output between -pi and pi, use -3.14159265, otherwise use 0.", "type": "number" }
    }
  })###";

String AngleCorrection::get_config_schema() { return FPSTR(SCHEMA); }

bool AngleCorrection::set_configuration(const JsonObject& config) {
  String expected[] = {"offset", "min_angle"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  offset_ = config["offset"];
  min_angle_ = config["min_angle"];
  return true;
}

}  // namespace sensesp
