#include "i2c_input.h"

#include "Arduino.h"
#include "Wire.h"

#include "sensesp.h"

int I2CInput::scanI2CAddress(uint8_t address) 
{
  uint8_t errorCode;
  
  Wire.beginTransmission(address);
  errorCode = Wire.endTransmission();
  if (errorCode == 0) {
    return true;
  }
  return false;
}

void I2CInput::scanAllI2C() {
  Serial.println(F("Scanning for i2c Sensors..."));
  for (int i=0; i < 0x7f; i++) {
    if (scanI2CAddress(i)) {
      Serial.printf("Found I2C device at address %x\n", i);
    }
  }
}

void I2CInput::setupI2C() 
{
  //Wire.setClock(0000L);

  Wire.begin();
  Wire.setClock(400000); // choose 400 kHz I2C rate
  
  scanAllI2C();
}


I2CInput::I2CInput(uint8_t address,String config_path) : NumericSensor() {
  className = "I2CInput";
  setupI2C() ;
  if(!scanI2CAddress(address))
    Serial.printf("Kein I2C Device an Adresse %x", address);

}

void I2CInput::update() 
{
  output = analogRead(A0);
  this->notify();
}

void I2CInput::enable() 
{
  app.onRepeat(100, [this](){ this->update(); });
}

