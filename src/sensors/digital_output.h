#ifndef _digital_output_H
#define _digital_output_H

#include <ArduinoJson.h>

#include "system/observable.h"
#include "transforms/transform.h"

/**
 * @brief Sets a GPIO pin to whatever the input is (true = HIGH,
 * false = LOW), and passes the value on to the next ValueConsumer.
 */
class DigitalOutput : public BooleanTransform {
 public:
  DigitalOutput(int pin);
  void set_input(bool new_value, uint8_t input_channel = 0) override;

 private:
  int pin_number_;
};

#endif