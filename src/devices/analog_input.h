#ifndef _analog_input_H_
#define _analog_input_H_

#include "device.h"

class AnalogInput : public NumericDevice {

public:
  void enable() override final;

private:
  void update();
};


#endif
