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

void convertFromJson(JsonVariantConst src, ENUVector &dst) {
  dst.east = src["east"].as<float>();
  dst.north = src["north"].as<float>();

  if (src.containsKey("up")) {
    dst.up = src["up"].as<float>();
  }
}

void convertFromJson(JsonVariantConst src, AttitudeVector &dst) {
  dst.roll = src["roll"].as<float>();
  dst.pitch = src["pitch"].as<float>();
  dst.yaw = src["yaw"].as<float>();
}

bool canConvertFromJson(JsonVariantConst src, const Position & /*position*/) {
  return src.containsKey("latitude") && src.containsKey("longitude");
}

bool canConvertFromJson(JsonVariantConst src, const ENUVector & /*enu*/) {
  return src.containsKey("east") && src.containsKey("north");
}

bool canConvertFromJson(JsonVariantConst src,
                        const AttitudeVector & /*attitude*/) {
  return src.containsKey("roll") && src.containsKey("pitch") &&
         src.containsKey("yaw");
}

void convertToJson(const Position &src, JsonVariant dst) {
  JsonObject obj = dst.to<JsonObject>();
  obj["latitude"] = src.latitude;
  obj["longitude"] = src.longitude;

  if (src.altitude != kPositionInvalidAltitude) {
    obj["altitude"] = src.altitude;
  }
}

void convertToJson(const ENUVector &src, JsonVariant dst) {
  JsonObject obj = dst.to<JsonObject>();
  obj["east"] = src.east;
  obj["north"] = src.north;

  if (src.up != kPositionInvalidAltitude) {
    obj["up"] = src.up;
  }
}

void convertToJson(const AttitudeVector &src, JsonVariant dst) {
  JsonObject obj = dst.to<JsonObject>();
  obj["roll"] = src.roll;
  obj["pitch"] = src.pitch;
  obj["yaw"] = src.yaw;
}

}  // namespace sensesp
