#ifndef _device_H_
#define _device_H_

#include <set>

#include "system/configurable.h"
#include "system/observable.h"
#include "system/valueproducer.h"


///////////////////
// Devices represent raw hardware thingies.

class Device : virtual public Observable, public Configurable {
 public:
  Device(String id="", String schema="");
  virtual void enable() {};
  static const std::set<Device*>& get_devices() {
    return devices;
  }
 private:
  static std::set<Device*> devices;
};



class NumericDevice : public Device, public NumericProducer {

    public:
        NumericDevice(String id="", String schema="", uint8_t valueIndex = 0);

};


class IntegerDevice : public Device, public IntegerProducer {

    public:
        IntegerDevice(String id="", String schema="", uint8_t valueIndex = 0);

};


class StringDevice : public Device, public StringProducer {

    public:
        StringDevice(String id="", String schema="", uint8_t valueIndex = 0);

};

#endif