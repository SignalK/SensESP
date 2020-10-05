#ifndef _digital_output_H
#define _digital_output_H

#include <ArduinoJson.h>

#include "system/observable.h"
#include "system/valueconsumer.h"

class DigitalOutput : public BooleanConsumer {
 public:
  DigitalOutput(int pin);
  void set_input(bool new_value, uint8_t input_channel = 0) override;

 private:
  int pin_number;
};

#endif