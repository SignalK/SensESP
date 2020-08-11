/*

    NASAClipper_I2C_to_SensESP v1.0
    A sensor for SensESP to decode the NASA Clipper Depth Data
    
    Written by Phil Shotton
    heavily based on code by Peter Holtermann
    with additions from Victor Klein

    This software is distributed under the GPL v3.0 License
    
    
    At the back of your NASA Clipper should be a round connector
    with 5 pins, looking roughly like this ASCII art:
      Pin Location Pin Number
          | 2nd
      \ / 4 5
    - - 1 3
          
          O 6
          
  Pin 1: SCL
  Pin 2: GND
  Pin 3: SDA
  Pin 4: 12V
  Pin 5: GND

  For detail refer also to:
  
  http://wiki.openseamap.org/wiki/De:NASA_Clipper_Range
  
  If you connect SCL, SDA and GND with your arduino
  and upload this sketch you should get the depth
  information sent to your local Signal K server.

*/
#include "nasa_depth.h"

static char i2c_data[11];
static bool new_data = false;


static void I2cDataHandler(int numBytes)
{
  if (numBytes !=11) {
    // should always get 11 bytes, error if not
    debugE("Received %d bytes from I2C but should have been 11", numBytes);
    return;
  }
  for (byte i = 0; i <numBytes; i++)
  {
    i2c_data[i] = Wire.read ();
  }
  new_data = true;
}

NasaDepth::NasaDepth(int8_t sda_pin, int8_t scl_pin, uint read_delay, String config_path) :
    NumericSensor(config_path), read_delay{read_delay} {
  className = "NasaDepth";
  load_configuration();
  Wire.begin(sda_pin, scl_pin);
  Wire.onReceive (&I2cDataHandler);
}

void NasaDepth::enable() {
  app.onRepeat(read_delay,[this]() {
    if (new_data) {
      output = DecodeData();
      if (output >= 0) {
        this->notify();
      }
    } 
  });
}

JsonObject& NasaDepth::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["read_delay"] = read_delay;
  root["value"] = output;
  return root;
};

static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "read_delay": { "title": "Read delay", "type": "number", "description": "Number of milliseconds between each thermocouple read " },
        "value": { "title": "Last value", "type" : "number", "readOnly": true }
    }
  })###";

String NasaDepth::get_config_schema() { return FPSTR(SCHEMA); }

bool NasaDepth::set_configuration(const JsonObject& config) {
  String expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  read_delay = config["read_delay"];
  return true;
}

// The NASA Clipper sends a 12 byte I2C data packet, this data is directly send to a pcf8566p
// LCD Driver. The first byte is the address and the write direction, the next eleven bytes is data.
// The first 5 bytes is a command, the next 6 bytes are data
// positions and contain the single LCD elements. 
// Example data {0x7c, 0xce, 0x80,0xe0,0xf8,0x70,0x00,0x00,0x00,0x00,0x00,0x00};
// addr 0 1 2 3 4 5 6 7 8 9 10
// com com com com com dta dta dta dta dta dta
// Example depth: 23.3
// Digit number: 12.3
float NasaDepth::DecodeData () 
{
  byte i, j;
  byte digit_tmp0, digit_tmp1, dig1, dig2, dig3, dec_point, index;
  char output_string[5];
  float output = -1;
  new_data = false;

    // Copy the I2C data
    for (j = 0; j <11; j++) {
      localCopy[j] = i2c_data[j];
      i2c_data[j] = 0;
    }
    // Check if the first 5 byte (the command) are correct
    // They seem to stay always the same
    for (i = 0; i <5; i++) {
      output_string[i] = 0;
      
      if ((localCopy[i] & 0xFF) != i2C_predata[i]) {
        debugE("Received invalid data at position %d", i);
        return output;
      }
    }
      // Decode the digits
      dig1 = 'N';
      dig2 = 'N';
      dig3 = 'N';
      dec_point = 'N';
  
       // DIGIT 3 
      digit_tmp0 = localCopy[6] & digit3_mask[1];
      for (i = 0; i <10; i++) {
        if ((digit3[i][1] & 0xFF) == (digit_tmp0 & 0xFF)) {
          dig3 = '0' + i;
          break;
        }
      }
      // decimal point
      if ((localCopy[8] & decpoint_mask[3] & 0xFF) == (0x80)) {
        dec_point = '.';
      }
      
      // We only consider data good, when the "DEPTH" symbol appears on the LCD
      if ((localCopy[5] & depth_mask[0] & 0xFF) != (0x01)) {
        debugE("No valid DEPTH symbol");
        return output;
      }
      
      
      // DIGIT 2 
      digit_tmp0 = localCopy[5] & digit2_mask[0];
      for (i = 0; i <10; i++) {
        if ((digit2[i][0] & 0xFF) == (digit_tmp0 & 0xFF)) {        
          dig2 = '0' + i;
          break;
        }
      }
      // DIGIT 1
      digit_tmp0 = localCopy[9] & digit1_mask[4];
      digit_tmp1 = localCopy[10] & digit1_mask[5];
      for (i = 0; i <10; i++) {
        if (((digit1[i][4] & 0xFF) == (digit_tmp0 & 0xFF)) &
        ((digit1[i][5] & 0xFF) == (digit_tmp1 & 0xFF))) {
          dig1 = '0' + i;
          break;
        }
      }
      
      i = 0;
      // Do we have good data? (FLAG_DEPTH and at least one digit
      if (((dig1 != 'N') || (dig2 != 'N') || (dig3 != 'N'))) {
        index = 0;
        if (dig1 != 'N') {
          output_string[index] = dig1;
          index++;
        }
        if (dig2 != 'N') {
          output_string[index] = dig2;
          index++;
        }
        if (dec_point != 'N') {
          output_string[index] = dec_point;
          index++;
        }
        if (dig3 != 'N') {
          output_string[index] = dig3;
          index++;
        }
        output_string[index] = '\0';
        Serial.println (output_string);
        sscanf(output_string, "%f", &output);

      } else {
        Serial.println ("$ bad data");
      }
    // Get rid of old localCopy
    for (i = 0; i <11; i++) {
      localCopy[i] = 0;
    }
    return output;  
}

 
