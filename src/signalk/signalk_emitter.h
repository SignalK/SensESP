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
