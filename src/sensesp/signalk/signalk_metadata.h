#ifndef SENSESP_SIGNALK_SIGNALK_METADATA_H_
#define SENSESP_SIGNALK_SIGNALK_METADATA_H_

#include <ArduinoJson.h>
#include <cmath>
#include <vector>

namespace sensesp {

/**
 * @brief Alarm state values for Signal K Zone metadata.
 * @see https://demo.signalk.org/documentation/_signalk/server-api/ALARM_STATE.html
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
 * @see https://demo.signalk.org/documentation/_signalk/server-api/Zone.html
 */
struct SKZone {
  float lower_;    ///< Lower bound of the zone; NAN if not set
  float upper_;    ///< Upper bound of the zone; NAN if not set
  String message_;
  SKAlarmState state_;

  SKZone(SKAlarmState state, const String& message,
         float lower = NAN, float upper = NAN)
      : lower_{lower}, upper_{upper}, message_{message}, state_{state} {}
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
  String example_;
  int supports_put_;            ///< -1 = not set, 0 = false, 1 = true
  float display_scale_lower_;   ///< NAN = not set
  float display_scale_upper_;   ///< NAN = not set
  std::vector<SKZone> zones_;

  /**
   * @param units The unit of measurement the value represents. See
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
   */
  SKMetadata(const String& units, const String& display_name = "",
             const String& description = "", const String& short_name = "",
             float timeout = -1.0);

  /// Default constructor creates a blank Metadata structure
  SKMetadata()
      : timeout_{-1},
        supports_put_{-1},
        display_scale_lower_{NAN},
        display_scale_upper_{NAN} {}

  /**
   * Adds an entry to the specified meta array that represents this metadata
   * definition. The default implementation adds a single object that contains
   * the member fields of this class as individual properties.
   * @param[in] sk_path The Signal K path this metadata represents
   * @param[out] meta The array the metadata entry is supposed to be added to
   */
  virtual void add_entry(const String& sk_path, JsonArray& meta);

 private:
  static const char* alarm_state_to_string(SKAlarmState state);
};

}  // namespace sensesp

#endif
