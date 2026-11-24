#ifndef SENSESP_SIGNALK_SIGNALK_METADATA_H_
#define SENSESP_SIGNALK_SIGNALK_METADATA_H_

#include <ArduinoJson.h>
#include <cmath>
#include <vector>

namespace sensesp {

/**
 * @brief Alarm state values for Signal K Zone metadata.
 * @see
 * https://demo.signalk.org/documentation/_signalk/server-api/ALARM_STATE.html
 */
enum class SKAlarmState {
  kNominal,
  kNormal,
  kAlert,
  kWarn,
  kAlarm,
  kEmergency
};

/**
 * @brief Represents a single Signal K alarm zone within metadata.
 *
 * A missing bound means the zone is unbounded on that side. Zones are
 * emitted in the order they were added; consumers differ in how they
 * resolve overlapping zones, so prefer non-overlapping zones.
 *
 * @see https://demo.signalk.org/documentation/_signalk/server-api/Zone.html
 */
struct SKMetadataZone {
  float lower;  ///< Lower bound of the zone; NAN if not set
  float upper;  ///< Upper bound of the zone; NAN if not set
  String message;
  SKAlarmState state;

  /**
   * @param state Alarm state the zone maps to.
   * @param message Text shown for values in the zone.
   * @param lower Lower bound; pass NAN for a zone only bounded from above.
   * @param upper Upper bound; omit for a zone only bounded from below.
   */
  SKMetadataZone(SKAlarmState state, const String& message, float lower,
                 float upper = NAN)
      : lower{lower}, upper{upper}, message{message}, state{state} {}
};

/**
 * @brief Holds Signal K meta data that is associated with
 * the sk_path an SKEmitter class may optionally send to the server.
 * According to the official Signal K specification: "All keys in the Signal K
 * specification must have a `description`, and where the key is a numeric value
 * it must have `units`". Thus, any paths that you output that are not
 * already part of the official Signal K specification should have these
 * two fields defined as metadata.
 * <p>Note that this structure is a subset of the complete meta model.
 * These are the most common properties. More complex strctures
 * can be created by creating a descendant class of SKMetadata and
 * overriding add_entry()
 * @see https://signalk.org/specification/1.5.0/doc/data_model_metadata.html
 * @see SKEmitter
 * @see SKOutput
 */
class SKMetadata {
 public:
  String display_name_;
  String units_;
  String description_;
  String short_name_;
  float timeout_;
  /// Advertises PUT support only. Register an SKPutRequestListener on the same
  /// path and connect it to the code that applies the requested value.
  bool supports_put_;
  String example_;  ///< Example value for the path, shown by consumers
  /// NAN = not set; displayScale is emitted only when both bounds are set
  float display_scale_lower_ = NAN;
  /// NAN = not set; displayScale is emitted only when both bounds are set
  float display_scale_upper_ = NAN;
  /// Alarm zones, emitted in the order they were added
  std::vector<SKMetadataZone> zones_;

  /**
   * @param units The unit of measurement the value represents. This is
   * primarily used for numeric values. If NO unit needs to be specified, pass
   * an empty string. See
   * https://github.com/SignalK/specification/blob/master/schemas/definitions.json#L87
   * @param display_name This is used on or near any display or gauge which
   * shows the data.
   * @param description This is the description for the Signal K path and must
   * always be the same as the description property within the Signal K Schema
   * for that path.
   * @param short_name Human readable name for the particular instance of this
   * value. The short version may be used by consumers where space is at a
   * premium
   * @param timeout Tells the consumer how long it should consider the value
   * valid. This value is specified in seconds. Specify -1.0 if you do not
   * want to specify a timeout.
   * @param supports_put Advertises that this path supports PUT requests.
   * Defaults to false. This metadata flag does not register a request handler:
   * also create an SKPutRequestListener for the same path and connect its
   * output to the code that applies the requested value.
   */
  SKMetadata(const String& units, const String& display_name = "",
             const String& description = "", const String& short_name = "",
             float timeout = -1.0, bool supports_put = false);

  /// Default constructor creates a blank Metadata structure
  SKMetadata() : timeout_{-1}, supports_put_{false} {}

  /**
   * Adds an entry to the specified meta array that represents this metadata
   * definition. The default implementation adds a single object that contains
   * the member fields of this class as individual properties.
   * @param[in] sk_path The Signal K path this metadata represents
   * @param[out] meta The array the metadata entry is supposed to be added to
   */
  virtual void add_entry(const String& sk_path, JsonArray& meta);

 protected:
  static const char* alarm_state_to_string(SKAlarmState state);
};

}  // namespace sensesp

#endif
