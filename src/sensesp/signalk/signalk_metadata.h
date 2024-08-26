#ifndef _signalk_metadata_H_
#define signalk_metadata_H_

#include <ArduinoJson.h>

namespace sensesp {

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
  SKMetadata() : timeout_{-1} {}

  /**
   * Adds an entry to the specified meta array that represents this metadata
   * definition. The default implementation adds a single object that contains
   * the member fields of this class as individual properties.
   * @param[in] sk_path The Signal K path this metadata represents
   * @param[out] meta The array the metadata entry is supposed to be added to
   */
  virtual void add_entry(const String& sk_path, JsonArray& meta);
};

}  // namespace sensesp

#endif
