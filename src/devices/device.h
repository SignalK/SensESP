#ifndef _device_H_
#define _device_H_

#include "../observable.h"


///////////////////
// Devices represent raw hardware thingies.

class Device : public Observable {
  public:
    virtual void enable() = 0;
};

#endif