#ifndef _Battery_H_
#define _Battery_H_

#include "sensors/sensor.h"
//#include <Adafruit_ADS1015.h>

#include "BLP/BLP.h" //BLP Battery LifePo4


class Battery {
public:
  Battery(uint8_t ADSVoltAddress = 0x48);
  //ADS1115* ads1115;
   BLP* blp_; //Battery Type LifePo4
};

class BatteryValue : public NumericSensor {

public:
enum BatteryValType { voltage, 
                      overvoltage, 
                      warnOvervoltage, 
                      undervoltage, 
                      warnUndervoltage, 
                      temperature, 
                      maxTemperature, 
                      warnTemperature, 
                      status, 
                      SOC, 
                      SOH, 
                      switchOn };

  BatteryValue(   Battery battery,
                  BatteryValType batteryValtype,
                  uint read_delay = 1000,
                  String config_path = "");
  
  void enable() override final;
  
  Battery* battery_;

private:
    BatteryValType val_type_;
    uint read_delay_;
    void get_configuration(JsonObject& doc) override;
    bool set_configuration(const JsonObject& config) override;
    String get_config_schema() override;
};

#endif