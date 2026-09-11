#ifndef SENSESP_SENSORS_ANALOG_READER_H_
#define SENSESP_SENSORS_ANALOG_READER_H_

#include "sensesp.h"

#include <cmath>

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
 public:
  /// The nominal 3.3 V supply voltage, in millivolts.
  static constexpr float kDefaultMaxVoltage = 3300.;

 protected:
  int pin_;
  float max_voltage_;

 public:
  /**
   * @param pin The GPIO pin to read.
   *
   * @param max_voltage The full-scale reference voltage, in millivolts. Must
   * be positive and finite; any other value would make read() return a
   * meaningless fraction, so it is rejected in favour of the default.
   */
  ESP32AnalogReader(int pin, float max_voltage = kDefaultMaxVoltage)
      : pin_{pin}, max_voltage_{max_voltage} {
    if (!(max_voltage > 0.) || !std::isfinite(max_voltage)) {
      max_voltage_ = kDefaultMaxVoltage;
      ESP_LOGE(__FILENAME__,
               "Invalid max_voltage %f mV; using the default %f mV instead",
               max_voltage, max_voltage_);
    }
  }

  float read() override { return analogReadMilliVolts(pin_) / max_voltage_; }
};
typedef ESP32AnalogReader AnalogReader;

}  // namespace sensesp

#endif
