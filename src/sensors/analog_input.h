#ifndef _analog_input_H_
#define _analog_input_H_

#include "analog_reader.h"
#include "sensor.h"

class AnalogInput : public NumericSensor {
 public:
  /**
   * @brief Sensor for reading the MCU analog input pins
   *
   * Read an analog input pin and return the result scaled onto a given range.
   * Note that the ADCs in both ESP8266 and ESP32 are quite unlinear and noisy
   * and unlikely to give very accurate results.
   *
   * @param[in] pin The GPIO pin to read. On ESP8266, always A0. On ESP32, at
   * the moment only ADC channel 1 (pins 32..39) is supported.
   * @param[in] read_delay Time delay between consecutive readings, in ms
   * @param[in] config_path Configuration path for the sensor
   * @param[in] output_scale The maximum scale of the output
   */
  AnalogInput(uint8_t pin = A0, uint read_delay = 200, String config_path = "",
              float output_scale = 1024.);
  void enable() override final;

 private:
  uint8_t pin;
  uint read_delay;
  float output_scale;
  BaseAnalogReader* analog_reader;
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
  void update();
};

#endif
