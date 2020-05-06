#ifndef _adds1x15_H_
#define _adds1x15_H_

#include <Wire.h>
#include <Adafruit_ADS1015.h>

#include "sensor.h"

// ADS1x15 creates an instance of an ADS1015 or ADS1115 analog-to-digital converter (ADC)
// that will be used by ADS1015channel and ADS1115channel. In main.cpp, you will use one of
// the type defined below: ADS1015 or ADS115.
template <class T_Ada_1x15>
class ADS1x15 : public Sensor {
 public:
  ADS1x15(uint8_t addr = 0x48, adsGain_t gain = GAIN_TWOTHIRDS, String config_path="");
  void enable() override final {}
  T_Ada_1x15* ads;
};

typedef ADS1x15<Adafruit_ADS1015> ADS1015;
typedef ADS1x15<Adafruit_ADS1115> ADS1115;


// ADS1015value is used to read the value of an ADS1x15 (either ADS1015 or ADS1115). If you want to read a single channel,
// make your channel parameter be 0, 1 2 or 3 and readADC_SingleEnded() will be used. If you want
// to read the difference between two channels, your channel parameter should be:
// - 10 will use readADC_Differential_0_1()
// - 23 will use readADC_Differential_2_3()
template <class T_ads_1x15>
class ADS1x15value : public NumericSensor {
  public:
    ADS1x15value(T_ads_1x15* ads1x15, uint8_t channel = 0, uint read_delay = 200, String config_path="");
    void enable() override final;

  private:
    T_ads_1x15* ads1x15;
    uint8_t channel;
    uint read_delay;
    virtual JsonObject& get_configuration(JsonBuffer& buf) override;
    virtual bool set_configuration(const JsonObject& config) override;
    virtual String get_config_schema() override;

};

typedef ADS1x15value<ADS1015> ADS1015value;
typedef ADS1x15value<ADS1115> ADS1115value;


#endif
