#include "max31856_thermocouple.h"

#include "Arduino.h"
#include "sensesp.h"

MAX31856Thermocouple::MAX31856Thermocouple(int8_t cs_pin, int8_t mosi_pin,
                                           int8_t miso_pin, int8_t clk_pin,
                                           int8_t drdy_pin,
                                           max31856_thermocoupletype_t tc_type,
                                           uint read_delay, String config_path)
    : NumericSensor(config_path),
      data_ready_pin_{drdy_pin},
      read_delay_{read_delay} {
  load_configuration();      
  max31856_ = new Adafruit_MAX31856(cs_pin, mosi_pin, miso_pin, clk_pin);
  if (!max31856_->begin()) {
    sensor_detected_ = false;
    debugW("No MAX31856 detected: check wiring.");
    return;
  }
  max31856_->setThermocoupleType(tc_type);
  max31856_->setConversionMode(MAX31856_ONESHOT_NOWAIT);
}

MAX31856Thermocouple::MAX31856Thermocouple(Adafruit_MAX31856* max31856,
                                           uint read_delay, String config_path)
    : NumericSensor(config_path),
      max31856_{max31856},
      read_delay_{read_delay} {
  load_configuration();
  max31856_->setConversionMode(MAX31856_ONESHOT_NOWAIT);
}

void MAX31856Thermocouple::enable() {
  // Must be at least 500 to allow time for temperature "conversion".
  if (!sensor_detected_) {
    debugE("MAX31856 not enabled: no sensor detected.");
    return;
  }  
  else {
    if (read_delay_ < 500) {
      read_delay_ = 500;
    }
    app.onRepeat(read_delay_, [this]() {
      max31856_->triggerOneShot();
      app.onDelay(450, [this]() {
        float temp = max31856_->readThermocoupleTemperature();
        this->emit(temp);
      });
    });
  } 
}

void MAX31856Thermocouple::get_configuration(JsonObject& root) {
  root["read_delay"] = read_delay_;
};

static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "read_delay": { "title": "Read delay", "type": "number", "description": "Number of milliseconds between each thermocouple read " }
    }
  })###";

String MAX31856Thermocouple::get_config_schema() { return FPSTR(SCHEMA); }

bool MAX31856Thermocouple::set_configuration(const JsonObject& config) {
  String expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  read_delay_ = config["read_delay"];
  return true;
}
