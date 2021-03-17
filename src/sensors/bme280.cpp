#include "bme280.h"

#include "sensesp.h"
#include <RemoteDebug.h>

// BME280 represents an ADAfruit (or compatible) BME280 temperature / pressure /
// humidity sensor.
BME280::BME280(uint8_t addr) : addr_{addr} {
  adafruit_bme280_ = new Adafruit_BME280();
  check_status();
}

void BME280::check_status() {
  bool started = adafruit_bme280_->begin(addr_);
  if (!started) {
    debugI(
        "Could not find a valid BME280 sensor. Check wiring, address, and "
        "sensor ID.");
    debugI("SensorID is: 0x%d", adafruit_bme280_->sensorID());
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
      bme280_{bme280},
      val_type_{val_type},
      read_delay_{read_delay} {
  load_configuration();
}

// BME280 outputs temp in Celsius. Need to convert to Kelvin before sending to
// Signal K. Pressure is output in Pascals, Humidity is output in relative
// humidity (0 - 100%) and needs to be converted to ratio (0-1).Dew point
// temperature is in kelvin and calculated from temperature and humidity.
void BME280Value::enable() {
  app.onRepeat(read_delay_, [this]() {
    if (val_type_ == temperature) {
      output = bme280_->adafruit_bme280_->readTemperature() +
               273.15;  // Kelvin is Celsius + 273.15
    } else if (val_type_ == pressure) {
      output = bme280_->adafruit_bme280_->readPressure();
    } else if (val_type_ == humidity) {
      output = bme280_->adafruit_bme280_->readHumidity()/100;
      } else if (val_type_ == dewPointTemperature) {

          // Dew point is calculated with Arden Buck Equation and Arden Buck valuation sets
          // For more info on the calculation see https://en.wikipedia.org/wiki/Dew_point#Calculating_the_dew_point

          float relative_humidity = bme280_->adafruit_bme280_->readHumidity()/100;  //humidity in percent, so divide by 100
          float temp_celcius = bme280_->adafruit_bme280_->readTemperature();    

          // valuation set for temperatures above 0°C
          float b = 17.368;
          float c = 238.88; 
          const float d = 234.5; 
           
          // valuation set for temperatures below 0°C
          if (temp_celcius < 0.0) {
            float b = 17.966;
            float c = 247.15; 
          }

          float gamma=log(relative_humidity * exp((b - (temp_celcius / d)) * (temp_celcius / (c + temp_celcius))));
          float dew_point = (c * gamma) / ( b-gamma );

          output = dew_point + 273.15; // Kelvin is Celsius + 273.15
       } else {

      output = 0.0;
    }

    notify();
  });
}

void BME280Value::get_configuration(JsonObject& root) {
  root["read_delay"] = read_delay_;
};

static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "read_delay": { "title": "Read delay", "type": "number", "description": "The time, in milliseconds, between each read of the input" }
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
  read_delay_ = config["read_delay"];
  return true;
}