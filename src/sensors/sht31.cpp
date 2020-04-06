#include "sht31.h"

#include "sensesp.h"
#include <RemoteDebug.h>


// SHT31 represents an ADAfruit (or compatible) SHT31 temperature & humidity sensor.
SHT31::SHT31(uint8_t addr, String config_path) :
       Sensor(config_path), addr{addr} {
    className = "SHT31";
    load_configuration();
    pAdafruitSHT31 = new Adafruit_SHT31();
    if (!pAdafruitSHT31->begin(addr)) {
       debugE("Could not find a valid SHT31 sensor: check address and wiring");
    }
}


// SHT31value reads and outputs the specified type of value of a SHT31 sensor
SHT31value::SHT31value(SHT31* pSHT31, SHT31ValType val_type, uint read_delay, String config_path) :
                   NumericSensor(config_path), pSHT31{pSHT31}, val_type{val_type}, read_delay{read_delay} {
      className = "SHT31value";
      load_configuration();
}

// SHT31 outputs temp in Celsius. Need to convert to Kelvin before sending to Signal K.
// Humidity is output in relative humidity (0 - 100%)
void SHT31value::enable() {
  app.onRepeat(read_delay, [this](){ 
      if (val_type == temperature) {
          output = pSHT31->pAdafruitSHT31->readTemperature() + 273.15; // Kelvin is Celsius + 273.15
      }
      else if (val_type == humidity) {
          output = pSHT31->pAdafruitSHT31->readHumidity();
      }
      else output = 0.0;
      
      notify();
 });
}

JsonObject& SHT31value::get_configuration(JsonBuffer& buf) {
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


  String SHT31value::get_config_schema() {
  return FPSTR(SCHEMA);
}

bool SHT31value::set_configuration(const JsonObject& config) {
  String expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  read_delay = config["read_delay"];
  return true;
}
