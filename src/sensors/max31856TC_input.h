#ifndef _MAX31856TC_input_H_
#define _MAX31856TC_input_H_
#include "sensor.h"
#include "Adafruit_MAX31856.h"

class MAX31856TC : public NumericSensor {
  public:
    MAX31856TC(int8_t cs_pin, int8_t mosi_pin, int8_t miso_pin, int8_t clk_pin, int8_t drdy_pin, max31856_thermocoupletype_t  tc_type, uint read_delay, String config_path = "");
    Adafruit_MAX31856* pAdafruitMAX31856;
    void enable() override final;

  private:
    int8_t dataRdy;
    uint read_delay;
    virtual JsonObject& get_configuration(JsonBuffer& buf) override;
    virtual bool set_configuration(const JsonObject& config) override;
    virtual String get_config_schema() override;

};

#endif
