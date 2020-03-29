#ifndef _i2c_input_H_
#define _i2c_input_H_

#include "sensor.h"

class I2CInput : public NumericSensor {

public:
    I2CInput(uint8_t given_address,uint8_t given_register, String config_path, uint8_t given_registersize);
  bool scanI2CAddress(uint8_t address);
  void enable() override final;
  void scanAllI2C();
  void setupI2C();
  virtual JsonObject& get_configuration(JsonBuffer& buf) override final;
  virtual bool set_configuration(const JsonObject& config) override final;
  virtual String get_config_schema() override;

private:
  bool found = true;
  uint8_t address;
  uint8_t regi;
  void update();
  void read_value();
  uint8_t registersize;
  bool PollI2C(uint8_t address, uint8_t register, uint8_t count);
  float ReadI2C(uint8_t address, uint8_t register, uint8_t count);
};


#endif
