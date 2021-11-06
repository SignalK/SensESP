#ifndef _signalk_listener_H_
#define _signalk_listener_H_

#include <ArduinoJson.h>

#include <set>

#include "Arduino.h"
#include "sensesp.h"
#include "system/configurable.h"
#include "system/observable.h"
#include "system/valueproducer.h"

namespace sensesp {

/**
 * @brief An Obervable class that listens for Signal K stream deltas
 * and notifies any observers of value changes. This serves as a
 * base class for code that needs to listen to the stream, with
 * the most common descendant being `SKValueListener`
 * @see SKValueListener
 */
class SKListener : virtual public Observable {
 public:
  /**
   * The constructor
   * @param sk_path The Signal K path that identifies
   * this particular subscription to value
   * @param listen_delay How often you want the SK Server to send the
   * data you're subscribing to
   */
  SKListener(String sk_path, int listen_delay);

  /**
   * Returns the current Signal K path. An empty string
   * is returned if this particular source is not configured
   * or intended to return actual data.
   */
  String& get_sk_path() { return sk_path; }

  int get_listen_delay() { return listen_delay; }

  virtual void parse_value(JsonObject& json) {}

  static const std::vector<SKListener*>& get_listeners() { return listeners; }

 protected:
  String sk_path;

 private:
  static std::vector<SKListener*> listeners;
  int listen_delay;
};

}  // namespace sensesp

#endif
