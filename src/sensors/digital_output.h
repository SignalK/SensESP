#ifndef _digital_output_H
#define _digital_output_H

#include <ArduinoJson.h>

#include "system/observable.h"
#include "transforms/transform.h"

/**
 * DigitalOutput configures the specified GPIO pin
 * as an output pin, then sets the status of that
 * pin to HIGH whenever set_input() is called
 * and new_value is true. The pin is set to LOW when
 * new_value is false. After the output pin's state
 * is set, the new_value is passed through unmodified
 * as the value returned by BooleanTransform::get().
 */
class DigitalOutput : public BooleanTransform {
 public:
  DigitalOutput(int pin);
  void set_input(bool new_value, uint8_t input_channel = 0) override;

 private:
  int pin_number;
};

#endif