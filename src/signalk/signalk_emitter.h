#ifndef _signalk_emitter_H_
#define _signalk_emitter_H_

#include <ArduinoJson.h>

#include <set>

#include "Arduino.h"
#include "sensesp.h"
#include "system/configurable.h"
#include "system/observable.h"
#include "system/valueproducer.h"

/**
 * A Signal K emitter is one that produces Signal K output to be
 * forwarded to the Signal K server (if the system is connected
 * to one).  Signal K is reported via the as_signalK() method.
 */
class SKEmitter : virtual public Observable {

 public:

  /**
   * Metadata holds optional Signal K meta data that is associated with
   * the sk_path the SKEmitter emits. If Metadata is defined, it will
   * be sent to the Signal K server along with the first delta emitted.
   * Note that this structure is a subset of the complete meta model.
   * These are the most common properties. More complex strctures
   * can be emitted by overriding add_metadata()
   * @see https://signalk.org/specification/1.5.0/doc/data_model_metadata.html
   * @see add_metadata()
   */
  struct Metadata {
     String display_name_;
     String units_;
     String description_;
     String short_name_;
     String timeout_;

     /**
      * @param display_name This is used on or near any display or gauge which shows the data.
      * @param units The unit of measurement the value represents. See https://github.com/SignalK/specification/blob/de83f65f0144bacde665be547d08eb9a12c70212/schemas/definitions.json#L82
      * @param description This is the description for the Signal K path and must always be the same as the description property within the Signal K Schema for that path.
      * @param short_name Human readable name for the particular instance of this value. The short version may be used by consumers where space is at a premium
      * @param timeout Tells the consumer how long it should consider the value valid. This value is specified in seconds.
      */
     Metadata(String display_name, String units = "", String description = "", String short_name = "", String timeout = "");
  };


  /**
   * The constructor
   * @param sk_path The Signal K path that identifies
   * this particular output
   */
  SKEmitter(String sk_path);

  /**
   * Returns the data to be reported to the server as
   * a Signal K json string.
   */
  virtual String as_signalk() { return "not implemented"; }


  /**
   * Returns a Metadata structure that describes the sk_path this SKEmitter
   * is associated with. If this emitter has no metadata, NULL is
   * returned.
   * @see add_metadata()
   */
  virtual Metadata* get_metadata() { return NULL; }


  /**
   * Adds this emitter's Signal K meta data to the specified
   * meta object. The default implementation adds the non-blank
   * fields found in the `Metadata` structure returned by
   * `get_metadata()`. If `get_metadata()` returns NULL, this
   * method does nothing. More complex meta data entries
   * can be constructed by overriding this method. 
   * @see get_metdata()
   * @see https://signalk.org/specification/1.5.0/doc/data_model_metadata.html
   */
  virtual void add_metadata(JsonArray& meta);


  /**
   * Returns the current Signal K path.  An empty string
   * is returned if this particular source is not configured
   * or intended to return actual data.
   */
  String& get_sk_path() { return sk_path; }

  void set_sk_path(const String& path) { sk_path = path; }

  static const std::vector<SKEmitter*>& get_sources() { return sources; }

 protected:
  String sk_path;

 private:
  static std::vector<SKEmitter*> sources;
};

#endif
