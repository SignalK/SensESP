#ifndef _sht31_H_
#define _sht31_H_

#include <Wire.h>
#include <Adafruit_SHT31.h>

#include "sensor.h"

// The SHT31 classes are based on the ADAfruit_SHT31 library.

// SHT31 represents an ADAfruit (or compatible) SHT31 temperature & humidity sensor.
// The constructor creates a pointer to the instance, and starts up the sensor. The pointer is
// passed to SHT31value, which retrieves the specified value.

class SHT31 : public Sensor {
  public:
    SHT31(uint8_t addr = 0x44, String config_path = "");
    Adafruit_SHT31* adafruit_sht31;

  private:
    uint8_t addr;
};


// Pass one of these in the constructor to SHT31value() to tell which type of value you want to output
enum SHT31ValType { temperature, humidity };

// SHT31Value reads and outputs the specified value of a SHT31 sensor.
class SHT31Value : public NumericSensor {
  public:
    SHT31Value(SHT31* sht31, SHT31ValType val_type, uint read_delay = 500, String config_path="");
    void enable() override final;
    SHT31* sht31;

  private:
    
    SHT31ValType val_type;
    uint read_delay;
    virtual void get_configuration(JsonObject& doc) override;
    virtual bool set_configuration(const JsonObject& config) override;
    virtual String get_config_schema() override;

};

[[deprecated("Use SHT31Value instead.")]]
typedef SHT31Value SHT31value;

#endif
