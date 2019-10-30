#ifndef _i2c_input_H_
#define _i2c_input_H_

#include "sensor.h"

class I2CInput : public NumericSensor {

public:
  int scanI2CAddress(uint8_t address) ;
  void scanAllI2C();
  void setupI2C();
  I2CInput(uint8_t address,String config_path="");
  void enable() override final;

private:
  void update();
};


#endif
