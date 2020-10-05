#include "max31856_thermocouple.h"

#include "Arduino.h"
#include "sensesp.h"

MAX31856Thermocouple::MAX31856Thermocouple(int8_t cs_pin, int8_t mosi_pin,
                                           int8_t miso_pin, int8_t clk_pin,
                                           int8_t drdy_pin,
                                           max31856_thermocoupletype_t tc_type,
                                           uint read_delay, String config_path)
    : NumericSensor(config_path),
      data_ready_pin{drdy_pin},
      read_delay{read_delay} {
  load_configuration();
  max31856 = new Adafruit_MAX31856(cs_pin, mosi_pin, miso_pin, clk_pin);
  if (!max31856->begin()) {
    while (1) delay(10);
  }
  max31856->setThermocoupleType(tc_type);
  max31856->setConversionMode(MAX31856_CONTINUOUS);
}

void MAX31856Thermocouple::enable() {
  app.onRepeat(read_delay, [this]() {
    while (digitalRead(data_ready_pin)) {
      delay(25);
    }
    float temp = max31856->readThermocoupleTemperature();
    output = temp;
    this->notify();
  });
}

void MAX31856Thermocouple::get_configuration(JsonObject& root) {
  root["read_delay"] = read_delay;
  root["value"] = output;
};

static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "read_delay": { "title": "Read delay", "type": "number", "description": "Number of milliseconds between each thermocouple read " },
        "value": { "title": "Last value", "type" : "number", "readOnly": true }
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
  read_delay = config["read_delay"];
  return true;
}
