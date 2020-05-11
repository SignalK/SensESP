#include "bme280.h"

#include "sensesp.h"
//#include "i2c_tools.h"
#include <RemoteDebug.h>


// BME280 represents an ADAfruit (or compatible) BME280 temperature / pressure / humidity sensor.
BME280::BME280(uint8_t addr, String config_path) :
       Sensor(config_path), addr{addr} {
    className = "BME280";
    load_configuration();
    pAdafruitBME280 = new Adafruit_BME280();
    check_status();
}


void BME280::check_status() {
  bool status = pAdafruitBME280->begin(addr);
  if (!status) {
        debugI("Could not find a valid BME280 sensor. Check wiring, address, and sensor ID.");
        debugI("SensorID is: 0x%d", pAdafruitBME280->sensorID());
        debugI("0xFF: is a BMP180 or BMP085, or a bad address");
        debugI("0x56-0x58 is a BMP280");
        debugI("0x60 is a BME280");
        debugI("0x61 is a BME680");
        while (1) delay(10);
    }
}


// BME280value reads and outputs the specified type of value of a BME280 sensor
BME280value::BME280value(BME280* pBME280, BME280ValType val_type, uint read_delay, String config_path) :
                   NumericSensor(config_path), pBME280{pBME280}, val_type{val_type}, read_delay{read_delay} {
      className = "BME280value";
      load_configuration();
}

// BME280 outputs temp in Celsius. Need to convert to Kelvin before sending to Signal K.
// Pressure is output in Pascals, Humidity is output in relative humidity (0 - 100%)
void BME280value::enable() {
  app.onRepeat(read_delay, [this](){ 
      if (val_type == temperature) {
          output = pBME280->pAdafruitBME280->readTemperature() + 273.15; // Kelvin is Celsius + 273.15
      }
      else if (val_type == pressure) {
          output = pBME280->pAdafruitBME280->readPressure();
      }  
      else if (val_type == humidity) {
          output = pBME280->pAdafruitBME280->readHumidity();
      }
      else output = 0.0;
      
      notify();
 });
}

JsonObject& BME280value::get_configuration(JsonBuffer& buf) {
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


  String BME280value::get_config_schema() {
  return FPSTR(SCHEMA);
}

bool BME280value::set_configuration(const JsonObject& config) {
  String expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  read_delay = config["read_delay"];
  return true;
}
