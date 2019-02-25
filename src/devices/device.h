#ifndef _device_H_
#define _device_H_

#include <set>

#include "system/configurable.h"
#include "system/observable.h"

///////////////////
// Devices represent raw hardware thingies.

class Device : public Observable, public Configurable {
 public:
  Device(String id="", String schema="");
  virtual void enable() = 0;
  static const std::set<Device*>& get_devices() {
    return devices;
  }
 private:
  static std::set<Device*> devices;
};

#endif