#ifndef _analog_input_H_
#define _analog_input_H_

#include "device.h"

class AnalogInput : public Device {
 public:
  void enable() override final;
  float get();
 private:
  float value;
  void update();
};

#endif
