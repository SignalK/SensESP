#include "angle_correction.h"

namespace sensesp {

constexpr double kPi = 3.14159265358979323846;

AngleCorrection::AngleCorrection(float offset, float min_angle,
                                 const String& config_path)
    : FloatTransform(config_path), offset_{offset}, min_angle_{min_angle} {
  load_configuration();
}

void AngleCorrection::set(const float& input) {
  // first the correction
  float x = input + offset_;

  // then wrap around the values
  x = fmod(x - min_angle_, 2 * kPi);
  if (x < 0) {
    x += 2 * kPi;
  }
  this->emit(x + min_angle_);
}

void AngleCorrection::get_configuration(JsonObject& root) {
  root["offset"] = offset_;
  root["min_angle"] = min_angle_;
}

static const char kSchema[] = R"###({
    "type": "object",
    "properties": {
        "offset": { "title": "Constant offset", "description": "Value to be added, in radians", "type": "number" },
        "min_angle": { "title": "Minimum angle value", "description": "If you have output between -pi and pi, use -3.14159265, otherwise use 0.", "type": "number" }
    }
  })###";

String AngleCorrection::get_config_schema() { return kSchema; }

bool AngleCorrection::set_configuration(const JsonObject& config) {
  String const expected[] = {"offset", "min_angle"};
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
