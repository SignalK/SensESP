#include "bmp280.h"

#include <RemoteDebug.h>

#include "sensesp.h"

// BMP280 represents an ADAfruit (or compatible) BMP280 temperature & pressure
// sensor.
BMP280::BMP280(uint8_t addr, String config_path, Adafruit_BMP280* sensor)
    : Sensor(config_path), addr_{addr} {
  load_configuration();
  adafruit_bmp280_ = sensor;
  if (!adafruit_bmp280_->begin(addr_)) {
    debugE("Could not find a valid BMP280 sensor: check address and wiring");
  }
}


// BMP280Value reads and outputs the specified type of value of a BMP280 sensor
BMP280Value::BMP280Value(BMP280* bmp280, BMP280ValType val_type,
                         uint read_delay, String config_path)
    : NumericSensor(config_path),
      bmp280_{bmp280},
      val_type_{val_type},
      read_delay_{read_delay} {
  load_configuration();
}

// BMP280 outputs temp in Celsius. Need to convert to Kelvin before sending to
// Signal K. Pressure is output in Pascals.
void BMP280Value::enable() {
  app.onRepeat(read_delay_, [this]() {
    if (val_type_ == temperature) {
      output = bmp280_->adafruit_bmp280_->readTemperature() +
               273.15;  // Kelvin is Celsius + 273.15
    } else if (val_type_ == pressure) {
      output = bmp280_->adafruit_bmp280_->readPressure();
    } else {
      output = 0.0;
    }

    notify();
  });
}

void BMP280Value::get_configuration(JsonObject& root) {
  root["read_delay"] = read_delay_;
};

static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "read_delay": { "title": "Read delay", "type": "number", "description": "The time, in milliseconds, between each read of the input" }
    }
  })###";

String BMP280Value::get_config_schema() { return FPSTR(SCHEMA); }

bool BMP280Value::set_configuration(const JsonObject& config) {
  String expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  read_delay_ = config["read_delay"];
  return true;
}
