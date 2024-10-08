#include "angle_correction.h"

namespace sensesp {

constexpr double kPi = 3.14159265358979323846;

AngleCorrection::AngleCorrection(float offset, float min_angle,
                                 const String& config_path)
    : FloatTransform(config_path), offset_{offset}, min_angle_{min_angle} {
  load();
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

bool AngleCorrection::to_json(JsonObject& root) {
  root["offset"] = offset_;
  root["min_angle"] = min_angle_;
  return true;
}

bool AngleCorrection::from_json(const JsonObject& config) {
  String const expected[] = {"offset", "min_angle"};
  for (auto str : expected) {
    if (!config[str].is<JsonVariant>()) {
      return false;
    }
  }
  offset_ = config["offset"];
  min_angle_ = config["min_angle"];
  return true;
}

const String ConfigSchema(const AngleCorrection& obj) {
  return R"###({"type":"object","properties":{"offset":{"title":"Constant offset","description":"Value to be added, in degrees","type":"number","displayMultiplier":0.017453292519943295,"displayOffset":0},"min_angle":{"title":"Minimum angle value","description":"Typically 0 or -180.","type":"number","displayMultiplier":0.017453292519943295,"displayOffset":0}}})###";
}

}  // namespace sensesp
