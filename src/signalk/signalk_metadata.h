#ifndef _signalk_metadata_H_
#define _signalk_metadata_H_

#include <ArduinoJson.h>

/**
 * @brief Holds Signal K meta data that is associated with
 * the sk_path an SKEmitter class may optionally send to the server.
 * Note that this structure is a subset of the complete meta model.
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
  String timeout_;

  /**
   * @param units The unit of measurement the value represents. See
   * https://github.com/SignalK/specification/blob/de83f65f0144bacde665be547d08eb9a12c70212/schemas/definitions.json#L82
   * @param display_name This is used on or near any display or gauge which
   * shows the data.
   * @param description This is the description for the Signal K path and must
   * always be the same as the description property within the Signal K Schema
   * for that path.
   * @param short_name Human readable name for the particular instance of this
   * value. The short version may be used by consumers where space is at a
   * premium
   * @param timeout Tells the consumer how long it should consider the value
   * valid. This value is specified in seconds.
   */
  SKMetadata(String units, String display_name = "", String description = "",
             String short_name = "", String timeout = "");

  /// Default constructor creates a blank Metadata structure
  SKMetadata() {}

  /**
   * Adds an entry to the specified meta array that represents this metadata
   * definition. The default implementation adds a single object that contains
   * the member fields of this class as individual properties.
   * @param[in] sk_path The Signal K path this metadata represents
   * @param[out] meta The array the metadata entry is supposed to be added to
   */
  virtual void add_entry(String sk_path, JsonArray& meta);
};

#endif