
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
  value["latitude"] = output_.latitude;
  value["longitude"] = output_.longitude;
  if (output_.altitude != kPositionInvalidAltitude) {
    value["altitude"] = output_.altitude;
  }
}

template <>
void SKOutput<ENUVector>::as_signalk_json(JsonDocument& doc) {
  doc["path"] = this->get_sk_path();
  JsonObject value = doc["value"].to<JsonObject>();
  value["east"] = output_.east;
  value["north"] = output_.north;
  if (output_.up != kPositionInvalidAltitude) {
    value["up"] = output_.up;
  }
}

template <>
void SKOutput<AttitudeVector>::as_signalk_json(JsonDocument& doc) {
  doc["path"] = this->get_sk_path();
  JsonObject value = doc["value"].to<JsonObject>();
  value["roll"] = output_.roll;
  value["pitch"] = output_.pitch;
  value["yaw"] = output_.yaw;
}

}  // namespace sensesp
