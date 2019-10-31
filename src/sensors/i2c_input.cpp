#include "i2c_input.h"

#include "Arduino.h"
#include "Wire.h"

#include "sensesp.h"

bool I2CInput::scanI2CAddress(uint8_t address) 
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


I2CInput::I2CInput(uint8_t given_address,String config_path) : NumericSensor(config_path) {
  className = "I2CInput";
  Serial.printf("I2C-Input "
  );
  address=given_address;
  load_configuration();
  setupI2C() ;
  if(!scanI2CAddress(address))
    {
      Serial.printf("Kein I2C Device an Adresse %x", address);
      found=false;
    }
    else
    {
      found=true;
    }
    

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

JsonObject& I2CInput::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root.set("value", output);
  root.set("address", address);
  root.set("found", found);
  return root;
}

static const char SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "address": { "title": "I2C address", "type": "number" },
        "found": { "title": "Device found", "type": "boolean", "readOnly": true },
        "value": { "title": "Last value", "type" : "number", "readOnly": true }
    }
  })";

String I2CInput::get_config_schema() {
  return FPSTR(SCHEMA);
}

bool I2CInput::set_configuration(const JsonObject& config) {
  if (!config.containsKey("address")) {
    return false;
  }
  address = config["address"];
  found== config["found"];
  return true;
}
