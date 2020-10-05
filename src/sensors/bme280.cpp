#include "bme280.h"

#include "sensesp.h"
//#include "i2c_tools.h"
#include <RemoteDebug.h>

// BME280 represents an ADAfruit (or compatible) BME280 temperature / pressure /
// humidity sensor.
BME280::BME280(uint8_t addr, String config_path)
    : Sensor(config_path), addr{addr} {
  load_configuration();
  adafruit_bme280 = new Adafruit_BME280();
  check_status();
}

void BME280::check_status() {
  bool started = adafruit_bme280->begin(addr);
  if (!started) {
    debugI(
        "Could not find a valid BME280 sensor. Check wiring, address, and "
        "sensor ID.");
    debugI("SensorID is: 0x%d", adafruit_bme280->sensorID());
    debugI("0xFF: is a BMP180 or BMP085, or a bad address");
    debugI("0x56-0x58 is a BMP280");
    debugI("0x60 is a BME280");
    debugI("0x61 is a BME680");
  }
}

// BME280Value reads and outputs the specified type of value of a BME280 sensor
BME280Value::BME280Value(BME280* bme280, BME280ValType val_type,
                         uint read_delay, String config_path)
    : NumericSensor(config_path),
      bme280{bme280},
      val_type{val_type},
      read_delay{read_delay} {
  load_configuration();
}

// BME280 outputs temp in Celsius. Need to convert to Kelvin before sending to
// Signal K. Pressure is output in Pascals, Humidity is output in relative
// humidity (0 - 100%)
void BME280Value::enable() {
  app.onRepeat(read_delay, [this]() {
    if (val_type == temperature) {
      output = bme280->adafruit_bme280->readTemperature() +
               273.15;  // Kelvin is Celsius + 273.15
    } else if (val_type == pressure) {
      output = bme280->adafruit_bme280->readPressure();
    } else if (val_type == humidity) {
      output = bme280->adafruit_bme280->readHumidity();
    } else
      output = 0.0;

    notify();
  });
}

void BME280Value::get_configuration(JsonObject& root) {
  root["read_delay"] = read_delay;
  root["value"] = output;
};

static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "read_delay": { "title": "Read delay", "type": "number", "description": "The time, in milliseconds, between each read of the input" },
        "value": { "title": "Last value", "type" : "number", "readOnly": true }
    }
  })###";

String BME280Value::get_config_schema() { return FPSTR(SCHEMA); }

bool BME280Value::set_configuration(const JsonObject& config) {
  String expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  read_delay = config["read_delay"];
  return true;
}
