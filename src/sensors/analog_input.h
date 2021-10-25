#ifndef _analog_input_H_
#define _analog_input_H_

#include "analog_reader.h"
#include "sensor.h"

  /**
   * @brief Sensor for reading the MCU analog input pins
   *
   * Read an analog input pin and return the result scaled onto a given range.
   * Note that the ADCs in both ESP8266 and ESP32 are quite nonlinear and noisy
   * and unlikely to give very accurate results. For accurate ADC results,
   * SensESP supports the ADS1015 and ADS1115 ADC's.
   *
   * @param[in] pin The GPIO pin to read. On ESP8266, always A0. On ESP32, at
   * the moment only ADC channel 1 (pins 32..39) is supported.
   *
   * @param[in] read_delay Time delay between consecutive readings, in ms
   *
   * @param[in] config_path Configuration path for the sensor
   *
   * @param[in] output_scale The scale of the converted input value that
   * AnalogInput produces. The maximum raw analog value returned by an ESP8266
   * and an ESP32 are different, but AnalogInput takes that into account by
   * converting them to a number between zero and 'output_scale'. This allows
   * the same code to run on both platforms. The default value for
   * 'output_scale' is 1024, which means output will be from 0 to 1023. If you
   * want your output to be on a different scale, use this parameter to indicate
   * the X in the `0 to X` scale. For example, if you want your output to be
   * expressed as a percentage (0 to 100), make this parameter be 100. If you want
   * your output to be the original voltage read by the AnalogIn pin, make this
   * parameter be the maximum voltage that can go into the pin (probably 3.3). If
   * you want your output to be the original voltage that was intput into a
   * physical voltage divider circuit before being read by the AnalogIn pin,
   * make this parameter be the maximum voltage that you would send into the
   * voltage divider circuit.
   */
 class AnalogInput : public FloatSensor {
 public:
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
