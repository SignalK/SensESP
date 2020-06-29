#include "max31856TC_input.h"
#include "Arduino.h"
#include "sensesp.h"

uint8_t dataRdy;


MAX31856TC::MAX31856TC(int8_t cs_pin, int8_t mosi_pin, int8_t miso_pin, int8_t clk_pin, int8_t drdy_pin, max31856_thermocoupletype_t  tc_type, String config_path) :
    Sensor(config_path) {
  className = "MAX31856TC";
  load_configuration();
  pAdafruitMAX31856 = new Adafruit_MAX31856(cs_pin, mosi_pin, miso_pin, clk_pin);
  if (!pAdafruitMAX31856->begin()) {
	  while(1) delay(10);
  }
  dataRdy = drdy_pin;
  pAdafruitMAX31856->setThermocoupleType(tc_type);
  pAdafruitMAX31856->setConversionMode(MAX31856_CONTINUOUS);
}

MAX31856TCvalue::MAX31856TCvalue(MAX31856TC* pMAX31856TC, uint read_delay, String config_path) :
  NumericSensor(config_path), pMAX31856TC{pMAX31856TC}, read_delay{read_delay} {
    className = "MAX31856TCvalue";
    load_configuration();
  }

void MAX31856TCvalue::enable() {
  app.onRepeat(read_delay, [this]() { 
      while (digitalRead(dataRdy)) {
		    delay(25);
	     }
	float temp = pMAX31856TC->pAdafruitMAX31856->readThermocoupleTemperature();
  output = temp;
  this->notify();
  });
}

JsonObject& MAX31856TCvalue::get_configuration(JsonBuffer& buf) {
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

String MAX31856TCvalue::get_config_schema() { return FPSTR(SCHEMA); }

bool MAX31856TCvalue::set_configuration(const JsonObject& config) {
  String expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  read_delay = config["read_delay"];
  return true;
}
