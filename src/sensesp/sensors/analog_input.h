#ifndef SENSESP_SENSORS_ANALOG_INPUT_H_
#define SENSESP_SENSORS_ANALOG_INPUT_H_

#include <memory>

#include "ReactESP.h"
#include "analog_reader.h"
#include "sensesp/ui/config_item.h"
#include "sensor.h"

namespace sensesp {

/**
 * @brief Sensor for reading the MCU analog input pins
 *
 * Read an analog input pin at regular intervals and return the result scaled
 * onto a given range. The reading is calibrated in millivolts using the
 * Arduino core's analogReadMilliVolts(), so it compensates for the ESP32 ADC's
 * nonlinearity. The ADCs are still noisy; for high accuracy, SensESP supports
 * the ADS1015 and ADS1115 ADCs.
 *
 * Unlike RepeatSensor, AnalogInput can expose its read delay in the web
 * configuration UI. Give it a config_path and pass it to ConfigItem() if you
 * want to adjust the read interval at run time.
 *
 * @param[in] pin The GPIO pin to read. Which pins are usable depends on the
 * ESP32 variant. Note that pins on ADC2 cannot be read while Wi-Fi is active,
 * which in practice limits you to ADC1 pins (GPIO 32..39 on the original
 * ESP32).
 *
 * @param[in] read_delay Time delay between consecutive readings, in ms
 *
 * @param[in] config_path Configuration path for the sensor
 *
 * @param[in] output_scale The scale of the converted input value that
 * AnalogInput produces. The raw value width of the ADC varies between ESP32
 * variants, so AnalogInput normalizes the calibrated input voltage against a
 * 3.3 V full-scale reference and multiplies that fraction by 'output_scale'.
 * Output is therefore in the range 0 to 'output_scale', and the same code
 * behaves identically across variants. The default value is 1024. If you want
 * your output expressed as a percentage, make this parameter 100. If you want
 * the actual voltage at the pin, make it 3.3 -- that holds regardless of the
 * ADC attenuation set with analogSetAttenuation(), because the reading is
 * calibrated in millivolts before it is scaled. If you want the voltage on the
 * input side of a physical voltage divider, make this parameter the voltage
 * that the divider maps onto the pin's full-scale range.
 */
class AnalogInput : public FloatSensor {
 public:
  AnalogInput(uint8_t pin = A0, unsigned int read_delay = 200,
              const String& config_path = "", float output_scale = 1024.);

  virtual ~AnalogInput() {
    if (repeat_event_ != nullptr) {
      repeat_event_->remove(event_loop());
    }
  }

  virtual bool to_json(JsonObject& root) override;
  virtual bool from_json(const JsonObject& config) override;

 protected:
  uint8_t pin_{};
  unsigned int read_delay_;
  float output_scale_;
  std::unique_ptr<BaseAnalogReader> analog_reader_{};
  reactesp::RepeatEvent* repeat_event_ = nullptr;
  void update();
};

const String ConfigSchema(AnalogInput& obj);

inline bool ConfigRequiresRestart(const AnalogInput& obj) { return true; }

}  // namespace sensesp

#endif
