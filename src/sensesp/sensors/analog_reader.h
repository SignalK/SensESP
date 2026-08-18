#ifndef SENSESP_SENSORS_ANALOG_READER_H_
#define SENSESP_SENSORS_ANALOG_READER_H_

#include "sensesp.h"

#include "Arduino.h"

namespace sensesp {

/**
 * @brief Used by AnalogInput as a hardware abstraction layer
 **/
class BaseAnalogReader {
 public:
  virtual ~BaseAnalogReader() = default;

  /**
   * @brief Read the input as a fraction of the full-scale reference voltage.
   *
   * @return A value in the range 0..1, where 1.0 corresponds to the reader's
   * full-scale reference voltage. Normalizing here keeps AnalogInput
   * independent of the ADC bit width, which varies between ESP32 variants.
   */
  virtual float read() = 0;
};

/**
 * @brief Calibrated analog reader for the ESP32 family.
 *
 * Reads the pin using the Arduino core's analogReadMilliVolts(), which applies
 * the ESP-IDF factory ADC calibration, and normalizes the result against a
 * reference voltage.
 */
class ESP32AnalogReader : public BaseAnalogReader {
 protected:
  int pin_;
  float max_voltage_;

 public:
  /**
   * @param pin The GPIO pin to read.
   *
   * @param max_voltage The full-scale reference voltage, in millivolts. The
   * default corresponds to the nominal 3.3 V supply voltage.
   */
  ESP32AnalogReader(int pin, float max_voltage = 3300.)
      : pin_{pin}, max_voltage_{max_voltage} {}

  float read() override { return analogReadMilliVolts(pin_) / max_voltage_; }
};
typedef ESP32AnalogReader AnalogReader;

}  // namespace sensesp

#endif
