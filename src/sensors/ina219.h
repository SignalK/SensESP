#ifndef _ina219_H_
#define _ina219_H_

#include <Wire.h>
#include <Adafruit_INA219.h>

#include "sensor.h"

// The INA219 classes are based on the ADAfruit_INA219 library.

// These are the used to tell the constructor what to set for maximum voltage and amperage.
// The default in the Adafruit constructor is 32V and 2A, so we only need to handle the other two.
enum INA219CAL_t { cal32_2, cal32_1, cal16_400 };

// INA219 represents an ADAfruit (or compatible) INA219 High Side DC Current Sensor.
// The constructor creates a pointer to the instance, and starts up the sensor. The pointer is
// passed to INA219value, which retrieves the specified value.
// 
class INA219 : public Sensor {
  public:
    INA219(uint8_t addr = 0x40, INA219CAL_t calibration_setting = cal32_2, String config_path = "");
    Adafruit_INA219* pAdafruitINA219;

};


// Pass one of these in the constructor to INA219value() to tell which type of value you want to output
enum INA219ValType { bus_voltage, shunt_voltage, current, power, load_voltage };

// INA219value reads and outputs the specified value of a INA219 sensor.
class INA219value : public NumericSensor {
  public:
    INA219value(INA219* pINA219, INA219ValType val_type, uint read_delay = 500, String config_path="");
    void enable() override final;
    INA219* pINA219;

  private:
    
    INA219ValType val_type;
    uint read_delay;
    virtual JsonObject& get_configuration(JsonBuffer& buf) override;
    virtual bool set_configuration(const JsonObject& config) override;
    virtual String get_config_schema() override;

};

#endif
