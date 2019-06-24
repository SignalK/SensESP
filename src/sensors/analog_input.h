#ifndef _analog_input_H_
#define _analog_input_H_

#include "sensor.h"

class AnalogInput : public NumericSensor {

public:
  AnalogInput();
  void enable() override final;

private:
  void update();
};


#endif
