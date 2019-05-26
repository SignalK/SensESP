#ifndef _device_H_
#define _device_H_

#include <set>

#include "system/configurable.h"
#include "system/observable.h"
#include "system/valueproducer.h"
#include "system/enable.h"


///////////////////
// Devices represent raw hardware thingies.

class Device : virtual public Observable, public Configurable, public Enable {
  public:
    Device(String id="", String schema="");

  // Was used by sensesp_app::enable(). Now handled by Enable.  Deprecate?
  static const std::set<Device*>& get_devices() {
    return devices;
  }

  private:
    static std::set<Device*> devices;
};



class NumericDevice : public Device, public NumericProducer {

    public:
        NumericDevice(String id="", String schema="");

};


class IntegerDevice : public Device, public IntegerProducer {

    public:
        IntegerDevice(String id="", String schema="");

};


class StringDevice : public Device, public StringProducer {

    public:
        StringDevice(String id="", String schema="");

};

#endif