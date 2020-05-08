#include "i2c_input.h"

#include "Arduino.h"
#include <Wire.h>

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


I2CInput::I2CInput(uint8_t given_address,uint8_t given_regi, String config_path, uint8_t given_registersize) : NumericSensor(config_path) {
  className = "I2CInput";
  Serial.printf("I2C-Input "
  );
  address=given_address;
  regi = given_regi;
  registersize = given_registersize;
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
  ReadI2C(address, 0, 31);
  if(registersize  == 8)
    output = ReadI2C(address, regi, 1);
  else
    output = ReadI2C(address, regi, 2);
  this->notify();
  /*
  {
    static int i=0;
  static clock_t old_t;
  clock_t new_t;
  new_t = millis();
  debugD("\nreg:%d-update: %dms\n", regi,millis());
  i++;
  //debugD("%s\n",output.c_str());
  
old_t = new_t;
}
*/
  // poll
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

bool I2CInput::PollI2C(uint8_t address, uint8_t register, uint8_t count)
{
    uint8_t errorCode;
    Wire.beginTransmission(address);
    Wire.write(0);  // the register
    errorCode = Wire.endTransmission();
    if (errorCode != 0) {
      Serial.print(F("Error pollling CMPS12 at address: "));
      Serial.print(address, HEX);
      Serial.print(F(" ErrorCode: "));
      Serial.println(errorCode);
      return(false);
    }
//  app.onDelay(100, this->ReadI2C(address, count));
  return(true);
}



float I2CInput::ReadI2C(uint8_t address, uint8_t regi, uint8_t count)
{
  uint8_t errorCode;
  int data[31] = { 0 };
  int i;
  int k;
  Wire.beginTransmission(address);
  Wire.write(regi);  // the register
  errorCode = Wire.endTransmission();
  if (errorCode != 0) 
      {
      Serial.print(F("Error reading from CMPS12 at address: "));
      Serial.print(address, HEX);
      Serial.print(F(" ErrorCode: "));
      Serial.println(errorCode);
      return(-1);
      }

  Wire.requestFrom(address, count); // either both int or both uint8_t...
  if (Wire.available() >= count)
    {
    //Serial.printf("\n%d gelesen von register %d\n",count, regi);
    for(i=0; i < count; i++)

      {
       data[i] = Wire.read();
       //Serial.printf("[%d]:%x,", i, data[i]);
      } 
    //Serial.printf("\n");
    }

  errorCode = Wire.endTransmission();
  if (errorCode != 0) 
      {
      Serial.print(F("Error reading from CMPS12 at address: "));
      Serial.print(address, HEX);
      Serial.print(F(" ErrorCode: "));
      Serial.println(errorCode);
      return(-1);
      }

 if(count == 2)
      {
        k= ((unsigned char)data[0])<<8;
        k+=(unsigned char)data[1];
        //Serial.printf("k HEX: %x  DEZ: %d DBL:%f\n\n", k,k,k);
      }
      else
      {
        k= (int)((char)data[0]);
        //Serial.printf("k HEX: %x  DEZ: %d DBL:%f\n\n", k,k,k);
        //Serial.printf("k HEX: %x  DEZ: %d DBL:%f\n\n", k,k,k);
      }
      
    // Convert the data
    //Serial.printf("\n Konvertiert: Hex: %x DEZ: %d\n", ((int16_t)(((uint16_t)data[2]<<8)+data[3])/10.0),((int16_t)(((uint16_t)data[2]<<8)+data[3])/10.0));   //OK
    return(k);   //OK
}


    /*
0   Software version
1   Bearing8
0x02, 0x03 Bearing_16   //  0-3599, representing 0-359.9 degrees. register 2 being the
                        //high byte. This is calculated by the processor from quaternion outputs of the BNO055
0x04    Pitch   //(+/- 90Â°)
0x05 Roll       //(+/- 90Â°)
0x06, 0x07 Magnetometer-X
0x08,0x09 Magnetometer-Y axis raw output, 16 bit signed integer (register 0x08 high byte)
0x0A,0x0B Magnetometer-Z axis raw output, 16 bit signed integer (register 0x0A high byte)
0x0C, 0x0D Accelerometer-X axis raw output, 16 bit signed integer (register 0x0C high byte)
0x0E, 0x0F Accelerometer-Y axis raw output, 16 bit signed integer (register 0x0E high byte)
0x10, 0x11 Accelerometer-Z axis raw output, 16 bit signed integer (register 0x10 high byte)
0x12, 0x13 Gyro-X axis raw output, 16 bit signed integer (register 0x12 high byte)
0x14, 0x15 Gyro-Y axis raw output, 16 bit signed integer (register 0x14 high byte)
0x16, 0x17 Gyro-Z axis raw output, 16 bit signed integer (register 0x16 high byte)
0x18,0x19 Temperature       //centigrade (register 0x18 high byte)
0x1A, 0x1B Bearing-16-Bosch// bit This is the angle Bosch generate in the BNO055 (0-5759),
                            //divide by 16 for degrees
0x1C, 0x1D Pitch-16         //(+/-180Â°)
0x1E Calibration-state      //, bits 0 and 1 reflect the calibration status (0 un-calibrated, 3 fully
                            //calibrated)
*/
    /*output[0] = data[0]; 
    output[1] = (data[1]/255.0)*360.0;//Bearing8
    output[2] = (int16_t)(((uint16_t)data[2]<<8)+data[3])/10;//OK
    ((data[2]<<8)+data[3])/10.0;//Bearing16
    output[3] = data[4];//Pich
    output[4] = data[5];//Roll
    output[5] = (int16_t)(((uint16_t)data[6]<<8)+data[7]);//MagX
    output[6] = (int16_t)(((uint16_t)data[8]<<8)+data[9]);//MagY
    output[7] = (int16_t)(((uint16_t)data[10]<<8)+data[11]);//MagZ
    output[8] = (int16_t)(((uint16_t)data[12]<<8)+data[13]);//Ax
    output[9] = (int16_t)(((uint16_t)data[14]<<8)+data[15]);//AY
    output[10] = (int16_t)(((uint16_t)data[16]<<8)+data[17]);//AZ
    output[11] = (int16_t)(((uint16_t)data[18]<<8)+data[19]);//GX
    output[12] = (int16_t)(((uint16_t)data[20]<<8)+data[21]);//GY
    output[13] = (int16_t)(((uint16_t)data[22]<<8)+data[23]);//GZ
    output[14] = (data[24]<<8)+data[25];//Temp
    output[15] = (int16_t)(((uint16_t)data[26]<<8)+data[27])/16.0;//Bearing16B
    output[16] = (int16_t)(((uint16_t)data[28]<<8)+data[29]);//Pitch16
    output[17] = data[30];//Calibration
*/
