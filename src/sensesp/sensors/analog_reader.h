#ifndef _analog_reader_H_
#define _analog_reader_H_

#include "Arduino.h"
#include "sensesp.h"
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
 private:
  int pin_;
  adc_atten_t attenuation_ = ADC_ATTEN_DB_11;
   // This should work with ESP32 and newer variants, ADCs are different
  adc_bits_width_t  bit_width_ = (adc_bits_width_t)  ADC_WIDTH_BIT_DEFAULT;
  // maximum voltage readout for 3.3V VDDA when attenuation_ is set to 11 dB
  const float kVmax_ = 3300;
  adc_channel_t adc_channel_;
  esp_adc_cal_characteristics_t adc_characteristics_;
  const int kVref_ = 1100;  // voltage reference, in mV
  adc_unit_t adc_unit; // ADC1 or ADC2
  bool config_successful = true;

 public:
  ESP32AnalogReader(int pin) : pin_{pin} {
    if (32 <= pin_ && pin_ <= 39) {
      adc_unit = ADC_UNIT_1;
      adc_channel_ = (adc_channel_t)digitalPinToAnalogChannel(pin_);
    }
    else {
      adc_unit = ADC_UNIT_2;
      if (pin_ == 13) adc_channel_ = ADC_CHANNEL_4;
      else if (pin_ == 25) adc_channel_ = ADC_CHANNEL_8;
      else if (pin_ == 26) adc_channel_ = ADC_CHANNEL_9;
      else if (pin_ == 27) adc_channel_ = ADC_CHANNEL_7;
    }
    
    if (!configure()) {
      config_successful = false;
    }
  }

 /**
   * @brief Configure (calibrate) the ADC for this specific ESP32
   * 
   * @return false if any of the configuration functions fail, otherwise true
   */
 
 bool configure() {
    if (adc_unit == ADC_UNIT_1) {
        if (adc1_config_width(bit_width_) != ESP_OK) {
          return false;
        }
        if (adc1_config_channel_atten((adc1_channel_t)adc_channel_, attenuation_) != ESP_OK) {
          return false;
        }
    } else {
        if (adc2_config_channel_atten((adc2_channel_t)adc_channel_, attenuation_) != ESP_OK) {
          return false;
        }
    }
    esp_adc_cal_characterize(adc_unit, attenuation_, bit_width_, kVref_,
                             &adc_characteristics_);
    return true;
  }

 float read() {
    uint32_t volts_mV;
    if (adc_unit == ADC_UNIT_1) {
      // returns the raw value of an adc1_channel_t and assigns it to volts_mV
      esp_adc_cal_get_voltage(adc_channel_, &adc_characteristics_,
                            &volts_mV);
      return (float)volts_mV / kVmax_;
    } 
    else { // ADC_UNIT_2
      int raw;
      // puts the raw value of an adc2_channel_t into raw
      adc2_get_raw((adc2_channel_t)adc_channel_, bit_width_, &raw);
      // convert raw adc reading to mV
      volts_mV = esp_adc_cal_raw_to_voltage(raw, &adc_characteristics_);
      return (float)volts_mV / kVmax_;
    }
  }

};
typedef ESP32AnalogReader AnalogReader;

}  // namespace sensesp

#endif
