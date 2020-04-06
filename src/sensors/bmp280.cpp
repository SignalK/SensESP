#include "bmp280.h"

#include "sensesp.h"
#include <RemoteDebug.h>


// BMP280 represents an ADAfruit (or compatible) BMP280 temperature & pressure sensor.
BMP280::BMP280(uint8_t addr, String config_path) :
       Sensor(config_path), addr{addr} {
    className = "BMP280";
    load_configuration();
    pAdafruitBMP280 = new Adafruit_BMP280();
    if (!pAdafruitBMP280->begin(addr)) {
       debugE("Could not find a valid BMP280 sensor: check address and wiring");
    }
}


// BMP280value reads and outputs the specified type of value of a BMP280 sensor
BMP280value::BMP280value(BMP280* pBMP280, BMP280ValType val_type, uint read_delay, String config_path) :
                   NumericSensor(config_path), pBMP280{pBMP280}, val_type{val_type}, read_delay{read_delay} {
      className = "BMP280value";
      load_configuration();
}

// BMP280 outputs temp in Celsius. Need to convert to Kelvin before sending to Signal K.
// Pressure is output in Pascals.
void BMP280value::enable() {
  app.onRepeat(read_delay, [this](){ 
      if (val_type == temperature) {
          output = pBMP280->pAdafruitBMP280->readTemperature() + 273.15; // Kelvin is Celsius + 273.15
      }
      else if (val_type == pressure) {
          output = pBMP280->pAdafruitBMP280->readPressure();
      }
      else output = 0.0;
      
      notify();
 });
}

JsonObject& BMP280value::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["read_delay"] = read_delay;
  root["value"] = output;
  return root;
  };

  static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "read_delay": { "title": "Read delay", "type": "number", "description": "The time, in milliseconds, between each read of the input" },
        "value": { "title": "Last value", "type" : "number", "readOnly": true }
    }
  })###";


  String BMP280value::get_config_schema() {
  return FPSTR(SCHEMA);
}

bool BMP280value::set_configuration(const JsonObject& config) {
  String expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  read_delay = config["read_delay"];
  return true;
}
