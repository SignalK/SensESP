#include "position.h"

#include "ArduinoJson.h"

namespace sensesp {

void convertFromJson(JsonVariantConst src, Position &dst) {
  dst.longitude = src["longitude"].as<double>();
  dst.latitude = src["latitude"].as<double>();

  if (src.containsKey("altitude")) {
    dst.altitude = src["altitude"].as<float>();
  }
}

bool canConvertFromJson(JsonVariantConst src, const Position & /*position*/) {
  return src.containsKey("latitude") && src.containsKey("longitude");
}

}  // namespace sensesp
