
#include "signalk_types.h"

namespace sensesp {

/**
 * @brief Template specialization for SKOutputPosition::as_signalk_json()
 *
 * This specialization allows `Position` objects to be output as Signal K
 * deltas.
 *
 * @tparam
 * @return String
 */
template <>
void SKOutput<Position>::as_signalk_json(JsonDocument& doc) {
  doc["path"] = this->get_sk_path();
  JsonObject value = doc["value"].to<JsonObject>();
  value["latitude"] = output.latitude;
  value["longitude"] = output.longitude;
  if (output.altitude != kPositionInvalidAltitude) {
    value["altitude"] = output.altitude;
  }
}

template <>
void SKOutput<ENUVector>::as_signalk_json(JsonDocument& doc) {
  doc["path"] = this->get_sk_path();
  JsonObject value = doc["value"].to<JsonObject>();
  value["east"] = output.east;
  value["north"] = output.north;
  if (output.up != kPositionInvalidAltitude) {
    value["up"] = output.up;
  }
}

template <>
void SKOutput<AttitudeVector>::as_signalk_json(JsonDocument& doc) {
  doc["path"] = this->get_sk_path();
  JsonObject value = doc["value"].to<JsonObject>();
  value["roll"] = output.roll;
  value["pitch"] = output.pitch;
  value["yaw"] = output.yaw;
}

}  // namespace sensesp
