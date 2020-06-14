#include "max31856TC_input.h"

#include "Arduino.h"
#include "sensesp.h"
#include "Adafruit_MAX31856.h"

Adafruit_MAX31856 max31856TC = Adafruit_MAX31856(SPI_CS_PIN, SPI_MOSI_PIN, SPI_MISO_PIN, SPI_CLK_PIN);


MAX31856TCInput::MAX31856TCInput(uint read_delay, String config_path)
    : NumericSensor(config_path), read_delay{read_delay} {
  className = "MAX31856TCInput";
  if (!max31856TC.begin()) {
	  while(1) delay(10);
  }
  max31856TC.setThermocoupleType(MAX31856_TCTYPE_K);
  max31856TC.setConversionMode(MAX31856_ONESHOT_NOWAIT);
  load_configuration();
}

void MAX31856TCInput::update() {
  output = tcRead();
  this->notify();
}

float MAX31856TCInput::tcRead() {
	max31856TC.triggerOneShot();
	delay(50);
	while (!max31856TC.conversionComplete()){
		delay(25);
	}
	float temp(max31856TC.readThermocoupleTemperature());
  Serial.print("TC Temp: ");
  Serial.println(temp);
  return temp;
}

void MAX31856TCInput::enable() {
  app.onRepeat(read_delay, [this]() { this->update(); });
}

JsonObject& MAX31856TCInput::get_configuration(JsonBuffer& buf) {
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

String MAX31856TCInput::get_config_schema() { return FPSTR(SCHEMA); }

bool MAX31856TCInput::set_configuration(const JsonObject& config) {
  String expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  read_delay = config["read_delay"];
  return true;
}
