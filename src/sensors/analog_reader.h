#ifndef _analog_reader_H_
#define _analog_reader_H_

#include "Arduino.h"
#include "sensesp.h"
#if defined(ESP32)
#include "esp_adc_cal.h"
#endif

class BaseAnalogReader {
 private:
  int output_scale_;

 public:
  virtual bool configure() = 0;
  virtual float read() = 0;
};

#ifdef ESP8266
class ESP8266AnalogReader : public BaseAnalogReader {
 private:
  uint8_t pin_;
  const unsigned int kAdcMax = 1024;

 public:
  ESP8266AnalogReader(uint8_t pin) : pin_{pin} { pinMode(pin, INPUT); }
  bool configure() { return true; }
  float read() { return (float)analogRead(pin_) / kAdcMax; }
};
typedef ESP8266AnalogReader AnalogReader;
#endif

#ifdef ESP32
class ESP32AnalogReader : public BaseAnalogReader {
 private:
  int pin_;
  adc_atten_t attenuation = ADC_ATTEN_DB_11;
  adc_bits_width_t bit_width = ADC_WIDTH_BIT_12;
  // maximum voltage readout for 3.3V VDDA when attenuation is set to 11 dB
  const float kVmax = 3300;
  int8_t adc_channel_;
  esp_adc_cal_characteristics_t adc_characteristics;
  const int kVref = 1100;  // voltage reference, in mV

 public:
  ESP32AnalogReader(int pin) : pin_{pin} {
    if (!(32 <= pin && pin <= 39)) {
      debugE("Only ADC1 is supported at the moment");
      adc_channel_ = -1;
      return;
    }
    adc_channel_ = digitalPinToAnalogChannel(pin);
  }

  bool configure() {
    if (adc_channel_ == -1) {
      return false;
    }
    adc1_config_width(bit_width);
    adc1_config_channel_atten((adc1_channel_t)adc_channel_, attenuation);
    esp_adc_cal_characterize(ADC_UNIT_1, attenuation, bit_width, kVref,
                             &adc_characteristics);
    return true;
  }

  float read() {
    uint32_t voltage;
    esp_adc_cal_get_voltage((adc_channel_t)adc_channel_, &adc_characteristics,
                            &voltage);
    return voltage / kVmax;
  }
};
typedef ESP32AnalogReader AnalogReader;
#endif

#endif