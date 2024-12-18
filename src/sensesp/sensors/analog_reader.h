#ifndef SENSESP_SENSORS_ANALOG_READER_H_
#define SENSESP_SENSORS_ANALOG_READER_H_

#include "sensesp.h"

#include "Arduino.h"

namespace sensesp {

/**
 * @brief Used by AnalogInput as a hardware abstraction layer
 **/
class BaseAnalogReader {
 private:
  int output_scale_;

 public:
  virtual float read() = 0;
};

class ESP32AnalogReader : public BaseAnalogReader {
 protected:
  int pin_;

 public:
  ESP32AnalogReader(int pin) : pin_{pin} {}

  float read() { return analogRead(pin_); }
};
typedef ESP32AnalogReader AnalogReader;

}  // namespace sensesp

#endif
