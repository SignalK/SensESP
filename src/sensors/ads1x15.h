#ifndef _adds1x15_H_
#define _adds1x15_H_

#include <Adafruit_ADS1015.h>
#include <Wire.h>

#include "sensor.h"


/**
 * @brief ADS1x15 represents an ADS1015 or ADS1115 Analog to Digital Converter.
 * 
 * In main.cpp, use one of these typedefs: ADS1015 or ADS1115
 *
 * @param addr The address through which the chip reports its readings. The default
 *   is 0x48, but can be changed on most chips - see the datasheet.
 * 
 * @param gain One of the adsGain_t values that determines the precision of
 *   the output. 
 *   See https://github.com/adafruit/Adafruit_ADS1X15/blob/master/Adafruit_ADS1015.h
 * 
 * @param config_path Not used for this Sensor, so you can omit it when you
 *   create an instance of this Class in main.cpp.
 * 
 * */
template <class T_Ada_1x15>
class ADS1x15 : public Sensor {
 public:
  ADS1x15(uint8_t addr = 0x48, adsGain_t gain = GAIN_TWOTHIRDS,
          String config_path = "");
  void enable() override final {}
  T_Ada_1x15* ads;
};

// define all possible instances of the class
typedef ADS1x15<Adafruit_ADS1015> ADS1015;
typedef ADS1x15<Adafruit_ADS1115> ADS1115;


/**
 * @brief ADS1x15RawValue is used to read the raw output of the ADS1x15.
 *   If you want the voltage that was the input to the ADS1x15 (voltage is
 *   what the ADS1x15 actually reads), use ADS1x15Voltage instead.
 *
 * @param ads1x15 A pointer to the ADS1x15 object.
 * 
 * @param channel The channel of the ADS1x15 that you want to read. For a
 *   single channel, use 0, 1, 2, or 3, and readADC_SingleEnded() will be used. If
 *   you want to read the difference between two channels, this parameter should be:
 *   - 10 to use readADC_Differential_0_1()
 *   - 23 to use readADC_Differential_2_3()
 * 
 * @param read_delay How often to read the value, in ms.
 * 
 * @param config_path The path to configuring read_delay in the Config UI.
 * 
 * */
template <class T_ads_1x15>
class ADS1x15RawValue : public NumericSensor {
 public:
  ADS1x15RawValue(T_ads_1x15* ads1x15, uint8_t channel = 0, uint read_delay = 200,
               String config_path = "");
  void enable() override final;

 private:
  T_ads_1x15* ads1x15;
  uint8_t channel;
  uint read_delay;
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
};

// define all possible instances of the class
typedef ADS1x15RawValue<ADS1015> ADS1015RawValue;
typedef ADS1x15RawValue<ADS1115> ADS1115RawValue;

// FIXME: Uncomment the following once the PIO Xtensa toolchain is updated
// [[deprecated("Use ADS1015RawValue instead.")]]
typedef ADS1015RawValue ADS1015value; // The original name
// FIXME: Uncomment the following once the PIO Xtensa toolchain is updated
// [[deprecated("Use ADS1015RawValue instead.")]]
typedef ADS1015RawValue ADS1015Value; // The second name
// FIXME: Uncomment the following once the PIO Xtensa toolchain is updated
// [[deprecated("Use ADS1115RawValue instead.")]]
typedef ADS1115RawValue ADS1115value; // The original name
// FIXME: Uncomment the following once the PIO Xtensa toolchain is updated
// [[deprecated("Use ADS1015RawValue instead.")]]
typedef ADS1115RawValue ADS1115Value; // The second name

#endif
