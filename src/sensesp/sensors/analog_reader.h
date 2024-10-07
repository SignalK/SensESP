#ifndef SENSESP_SENSORS_ANALOG_READER_H_
#define SENSESP_SENSORS_ANALOG_READER_H_

#include "sensesp.h"

#include "Arduino.h"
#if defined(ESP32)
#include "esp_adc_cal.h"
#endif

namespace sensesp {

/**
 * @brief Used by AnalogInput as a hardware abstraction layer
 **/
class BaseAnalogReader {
 private:
  int output_scale_;

 public:
  virtual bool configure() = 0;
  virtual float read() = 0;
};

class ESP32AnalogReader : public BaseAnalogReader {
 protected:
  int pin_;
  adc_atten_t attenuation_ = ADC_ATTEN_DB_12;
  // This should work with ESP32 and newer variants, ADCs are different
  adc_bits_width_t bit_width_ = (adc_bits_width_t)ADC_WIDTH_BIT_DEFAULT;
  // maximum voltage readout for 3.3V VDDA when attenuation_ is set to 11 dB
  const float kVmax_ = 3300;
  int8_t adc_channel_;
  esp_adc_cal_characteristics_t adc_characteristics_;
  const int kVref_ = 1100;  // voltage reference, in mV

 public:
  ESP32AnalogReader(int pin) : pin_{pin} {
    if (!(32 <= pin && pin <= 39)) {
      ESP_LOGE(__FILENAME__, "Only ADC1 is supported at the moment");
      adc_channel_ = -1;
      return;
    }
    adc_channel_ = digitalPinToAnalogChannel(pin);
  }

  bool configure() {
    if (adc_channel_ == -1) {
      return false;
    }
    adc1_config_width(bit_width_);
    adc1_config_channel_atten((adc1_channel_t)adc_channel_, attenuation_);
    esp_adc_cal_characterize(ADC_UNIT_1, attenuation_, bit_width_, kVref_,
                             &adc_characteristics_);
    return true;
  }

  float read() {
    uint32_t voltage;
    esp_adc_cal_get_voltage((adc_channel_t)adc_channel_, &adc_characteristics_,
                            &voltage);
    return voltage / kVmax_;
  }
};
typedef ESP32AnalogReader AnalogReader;

}  // namespace sensesp

#endif
