#ifndef _device_H_
#define _device_H_

#include "system/configurable.h"
#include "system/observable.h"

///////////////////
// Devices represent raw hardware thingies.

// TODO: devices should register themselves

class Device : public Observable, public Configurable {
 public:
  Device(String id="", String schema="") : Configurable{id, schema} {}
  virtual void enable() = 0;
};

#endif