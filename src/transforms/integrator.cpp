#include "integrator.h"

#include "sensesp_app.h"

// Integrator

Integrator::Integrator(float k, float value, String config_path)
    : NumericTransform(config_path), k{k}, value{value} {
  load_configuration();
}

void Integrator::enable() {
  // save the integrator value every 10 s
  // NOTE: Disabled for now because interrupts start throwing
  // exceptions.
  // app.onRepeat(10000, [this](){ this->save_configuration(); });
}

void Integrator::set_input(float input, uint8_t inputChannel) {
  value += input * k;
  this->emit(value);
}

void Integrator::reset() {value = 0;}

void Integrator::get_configuration(JsonObject& root) {
  root["k"] = k;
  root["value"] = value;
}

static const char SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "k": { "title": "Multiplier", "type": "number" },
        "value": { "title": "Current value", "type" : "number", "readOnly": false }
    }
  })";

String Integrator::get_config_schema() { return FPSTR(SCHEMA); }

bool Integrator::set_configuration(const JsonObject& config) {
  String expected[] = {"k"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  k = config["k"];
  value = config["value"];
  return true;
}
