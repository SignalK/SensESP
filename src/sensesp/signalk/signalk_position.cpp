
#include "signalk_position.h"

namespace sensesp {

/**
 * @brief Template specialization for SKOutputPosition::as_signalk()
 *
 * This specialization allows `Position` objects to be output as Signal K
 * deltas.
 *
 * @tparam
 * @return String
 */
template <>
String SKOutput<Position>::as_signalk() {
  DynamicJsonDocument json_doc(1024);
  String json;
  json_doc["path"] = this->get_sk_path();
  JsonObject value = json_doc.createNestedObject("value");
  value["latitude"] = output.latitude;
  value["longitude"] = output.longitude;
  if (output.altitude != kPositionInvalidAltitude) {
    value["altitude"] = output.altitude;
  }
  serializeJson(json_doc, json);
  return json;
}

}  // namespace sensesp
