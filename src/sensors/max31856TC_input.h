#ifndef _MAX31856TC_input_H_
#define _MAX31856TC_input_H_

#define SPI_CS_PIN 15
#define SPI_MOSI_PIN 13
#define SPI_MISO_PIN 12
#define SPI_CLK_PIN 14

//#define MAX_TCTYPE MAX_TCTYPE_K // other types can be B, E, J, N, R, S, T

#include "sensor.h"

class MAX31856TCInput : public NumericSensor {
 public:
  MAX31856TCInput(uint read_delay = 200, String config_path = "");
  void enable() override final;

 private:
  uint read_delay;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
  void update();
  float tcRead();
};

#endif
