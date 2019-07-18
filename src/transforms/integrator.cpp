#include "integrator.h"

#include "sensesp_app.h"

// Integrator

Integrator::Integrator(float k, float value, String config_path) :
    NumericTransform(config_path),
      k{ k } {
  className = "Integrator";
  output = value;
  load_configuration();
}


void Integrator::enable() {
  // save the integrator value every 10 s
  // NOTE: Disabled for now because interrupts start throwing
  // exceptions.
  //app.onRepeat(10000, [this](){ this->save_configuration(); });
}

void Integrator::set_input(float input, uint8_t inputChannel) {
  output += input;
  notify();
}


JsonObject& Integrator::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["k"] = k;
  root["value"] = output;
  return root;
}

static const char SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "k": { "title": "Multiplier", "type": "number" },
        "value": { "title": "Current value", "type" : "number", "readOnly": false }
    }
  })";

String Integrator::get_config_schema() {
  return FPSTR(SCHEMA);
}

bool Integrator::set_configuration(const JsonObject& config) {
  String expected[] = {"k" };
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  k = config["k"];
  output = config["value"];
  return true;
}
