#ifndef _MAX31856_thermocouple_H_
#define _MAX31856_thermocouple_H_
#include "Adafruit_MAX31856.h"
#include "sensor.h"

class MAX31856Thermocouple : public NumericSensor {
 public:
  MAX31856Thermocouple(int8_t cs_pin, int8_t mosi_pin, int8_t miso_pin,
                       int8_t clk_pin, int8_t drdy_pin,
                       max31856_thermocoupletype_t tc_type, uint read_delay,
                       String config_path = "");
  Adafruit_MAX31856* max31856;
  void enable() override final;

 private:
  int8_t data_ready_pin;
  uint read_delay;
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
};

#endif
