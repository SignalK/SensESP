#ifndef SENSESP_SENSORS_DIGITAL_PCNT_INPUT_H_
#define SENSESP_SENSORS_DIGITAL_PCNT_INPUT_H_

#include <elapsedMillis.h>

#include "sensesp/sensors/digital_input.h"
#include "sensesp/sensors/sensor.h"
#include "sensesp_base_app.h"
#if ESP_ARDUINO_VERSION_MAJOR < 3
#warning \
    "The Pulse Counter API is only available in ESP32 Arduino Core 3.0.0 and later"
#else
#include "driver/pulse_cnt.h"

namespace sensesp {

/**
 * @brief DigitalInputPcntCounter is the base class for reading
 * a digital GPIO pin using the Pulse Counter peripheral.
 *
 * A typical use case is to count the revolutions of something, such as an
 * engine, to determine RPM. See /examples/rpm_counter.cpp
 *
 * @param pin The GPIO pin to which the device is connected
 *
 * @param pin_mode Will be INPUT or INPUT_PULLUP
 *
 * @param interrupt_type Accepted values be RISING, FALLING, CHANGE see
 * Arduino.h
 *
 * @param read_delay How often you want to read the pin, in ms
 *
 * @param config_path The path to configuring read_delay in the Config UI
 *
 * */
class DigitalInputPcntCounter : public DigitalInput, public Sensor<int> {
 public:
  DigitalInputPcntCounter(uint8_t pin, int pin_mode, int interrupt_type,
                          unsigned int read_delay, String config_path = "");

  ~DigitalInputPcntCounter();

  virtual bool to_json(JsonObject& root) override;
  virtual bool from_json(const JsonObject& config) override;

 protected:
  unsigned int read_delay_;

 private:
  esp_err_t configurePcnt(int interrupt_type);
  pcnt_unit_handle_t pcnt_unit_ = nullptr;
  pcnt_channel_handle_t pcnt_channel_ = nullptr;

  int interrupt_type_;
};

const String ConfigSchema(const DigitalInputPcntCounter& obj);

bool ConfigRequiresRestart(const DigitalInputPcntCounter& obj);
}  // namespace sensesp

#endif  // ESP_ARDUINO_VERSION_MAJOR < 3
#endif  // SENSESP_SENSORS_DIGITAL_PCNT_INPUT_H_
