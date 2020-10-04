/*
This sensor outputs a voltage unlike the AnalogInput which ouputs a raw ADC value 

This sensor is required to deal with the specific setup of the ESP32 ADC's.

The decisions are:
1) Whether to use ADC1 or ADC2.

This code does not support ADC2 because the use of ADC2 requires interaction with the WiFi 
code as ADC2 is shared with the wifi driver.

The pins associated with ADC1 are 32 to 39.

If the internal Hall sensor is used then pins 36 & 39 should not be used. 
Pins 36 & 39 are also associated with the power switching of ADC2 for wifi and will have 
short "pull down" events of 89nS. While this is probably not critical to most applications 
its advisable to only use these pins if no others are available.

2) The next decision is what resolution is required for your application. The resolution 
ranges from 9 bits (512) to 12 bits (4096). In most cases the highest resolution would be 
applicable. The default is 12 bits.

Note that the ESP32-S2 allows only a 13 bit (8192) resolution setting.
!! NB When using the ESP32-S2 the default of 12 bits must be overidden with 
   ADC_WIDTH_13Bit !!

3) The most critical consideration is the attenuation option. The ESP32 ADC's allow the user 
to apply an attenuator in order to increase the "full-scale range" of the input from 1.1v to
3.9V.

This obviously impacts on the resolution of the reading. For example, if your input is never 
going to exceed 950mV then you would set for zero attenuation. The selected resolution 
would apply to the range corresponding with the selected attenuation.

If your range exceeds 950mV then you have the option to apply attenuation as per the diagram 
below. The ability to set attenuation also may have an impact on the accuracy of your 
readings as per this diagram:

    +----------+------------+--------------------------+
    |   SoC    | attenuation|   suggested range (mV)   |
    +==========+============+==========================+
    |          |  0dB       |     100 ~ 950            |
    |          +------------+--------------------------+
    |          |  2.5dB     |     100 ~ 1250           |
    |   ESP32  +------------+--------------------------+
    |          |  6dB       |     150 ~ 1750           |
    |          +------------+--------------------------+
    |          |  11dB      |     150 ~ 2450           |
    +----------+------------+--------------------------+
    |          |  0dB       |     100 ~ 800            |
    |          +------------+--------------------------+
    |          |  2.5dB     |     100 ~ 1100           |
    | ESP32-S2 +------------+--------------------------+
    |          |  6dB       |     150 ~ 1350           |
    |          +------------+--------------------------+
    |          |  11dB      |     150 ~ 2600           |
    +----------+------------+--------------------------+

The default is 11db attenuation (ADC_ATTEN_DB_11)

The diagram above indicates that using the portion of the range indicated is highly
recommended. The careful selection of reference/volatage divider resistors in conjunction 
with attenuation settings can greatly improve the accuracy of the ESP32 analog input.

The main difference between ESP32 and ESP8266 is the requirement to "characterise" the ADC.
This is essentially an internal calibration of the ADC using the selected ADC resolution, 
selected ADC attenuation and the voltage reference which can be a nominal 1100mV value, a
single point coefficient value stored in eFuse memory or a two point reference coefficients 
also stored in eFuse memory. A calibration call must be made prior to calling any read 
functions.

The lack of applying the characterisation of the ADC has greatly contributed to the bad rap 
of the ESP32 for analog input.

*/



#ifndef _esp32_analog_input_H_
#define _esp32_analog_input_H_

#include "sensor.h"
#include "esp_adc_cal.h"

#define V_REF 1100

class ESP32AnalogInput : public NumericSensor {

public:
  ESP32AnalogInput(uint8_t pin = 32, adc_bits_width_t adc_width = ADC_WIDTH_12Bit, 
              adc_atten_t adc_attenuation = ADC_ATTEN_11db, uint read_delay = 200, 
              String config_path = "");
  void enable() override final;

private:
  uint8_t pin;
  uint read_delay;
 	int adc_width = ADC_WIDTH_BIT_12;
	int adc_attenuation = ADC_ATTEN_DB_11;
	esp_adc_cal_characteristics_t characteristics;
	adc_channel_t adc_channel ;  // for reading voltage
  adc1_channel_t adc1_channel; // for configuring the ADC1 channel
	boolean configured = false;
  esp_adc_cal_value_t cal_type;  
  String cal_type_str;
  
  // cal_type holds the type of calibration - Two point, eFuse Vref or default vref

 	void associatePin(int pin, adc_bits_width_t adc_width, adc_atten_t adc_attenuation);
  float readVolts();

  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
  void update();
};

#endif
