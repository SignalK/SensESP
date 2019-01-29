#ifndef _analog_input_H_
#define _analog_input_H_

#include "device.h"

class AnalogInput : public Device {
    float value;
    void update();
  public:
    void enable() override final;
    float get();
};

#endif
