#ifndef _digital_output_H
#define _digital_output_H

#include <ArduinoJson.h>

#include "system/observable.h"
#include "transforms/transform.h"

namespace sensesp {

/**
 * @brief Sets a GPIO pin to whatever the input is (true = HIGH,
 * false = LOW), and passes the value on to the next ValueConsumer.
 *
 * @param pin Pin number of the pin you want to output to.
 */
class DigitalOutput : public BooleanTransform {
 public:
  DigitalOutput(int pin);
  void set_input(bool new_value, uint8_t input_channel = 0) override;

 private:
  int pin_number_;
};

}  // namespace sensesp

#endif
