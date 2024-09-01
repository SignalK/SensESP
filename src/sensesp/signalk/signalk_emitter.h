#ifndef SENSESP_SIGNALK_SIGNALK_EMITTER_H_
#define SENSESP_SIGNALK_SIGNALK_EMITTER_H_

#include "sensesp.h"

#include <ArduinoJson.h>
#include <set>

#include "sensesp/system/configurable.h"
#include "sensesp/system/observable.h"
#include "sensesp/system/valueproducer.h"
#include "signalk_metadata.h"

namespace sensesp {

/**
 * @brief A class that produces Signal K output to be
 * forwarded to the Signal K server (if the system is connected
 * to one).  Signal K is reported via the as_signalK() method.
 */
class SKEmitter : virtual public Observable {
 public:
  /**
   * The constructor
   * @param sk_path The Signal K path that identifies
   * this particular output
   */
  SKEmitter(const String& sk_path);

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
  virtual SKMetadata* get_metadata() { return NULL; }

  /**
   * Adds this emitter's Signal K meta data to the specified
   * meta object. The default implementation calls the
   * `add_entry()` method of the `SKMetadata` class returned by
   * `get_metadata()`. If `get_metadata()` returns NULL, this
   * method does nothing.
   * @see get_metdata()
   */
  virtual void add_metadata(JsonArray& meta);

  /**
   * Returns the current Signal K path.  An empty string
   * is returned if this particular source is not configured
   * or intended to return actual data.
   */
  String& get_sk_path() { return sk_path_; }

  void set_sk_path(const String& path) { sk_path_ = path; }

  static const std::vector<SKEmitter*>& get_sources() { return sources_; }

 protected:
  String sk_path_{};

 private:
  static std::vector<SKEmitter*> sources_;
};

}  // namespace sensesp

#endif
