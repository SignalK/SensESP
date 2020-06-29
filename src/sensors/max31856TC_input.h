#ifndef _MAX31856TC_input_H_
#define _MAX31856TC_input_H_
#include "sensor.h"
#include <Adafruit_MAX31856.h>

class MAX31856TC : public Sensor {
  public:
    MAX31856TC(int8_t cs_pin, int8_t mosi_pin, int8_t miso_pin, int8_t clk_pin, int8_t drdy_pin, max31856_thermocoupletype_t  tc_type, String config_path = "");
    Adafruit_MAX31856* pAdafruitMAX31856;
};

class MAX31856TCvalue : public NumericSensor {
  public:
    MAX31856TCvalue(MAX31856TC* pMAX31856TC, uint read_delay = 500, String config_path = "");
    void enable() override final;
    MAX31856TC* pMAX31856TC;

 private:

  uint read_delay;
  uint8_t dataRdy;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
//  void update();
};

#endif
