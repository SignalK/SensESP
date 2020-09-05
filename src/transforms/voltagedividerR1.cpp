#include "voltagedividerR1.h"

VoltageDividerR1::VoltageDividerR1(float R2, float Vin, String config_path) :
   SymmetricTransform<float>(config_path ), R2{R2}, Vin{Vin} {
     className = "VoltageDividerR1";
     load_configuration();
}

void VoltageDividerR1::set_input(float Vout, uint8_t ignored) {
    output = (Vin - Vout) * R2 / Vout;
    notify();
}


JsonObject& VoltageDividerR1::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["Vin"] = Vin;
  root["R2"] = R2;
  return root;
}

static const char SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "Vin": { "title": "Voltage in", "type": "number" },
        "R2": { "title": "Resistance (ohms) of R2", "type": "number" }
    }
  })";

String VoltageDividerR1::get_config_schema() {
  return FPSTR(SCHEMA);
}

bool VoltageDividerR1::set_configuration(const JsonObject& config) {

  String expected[] = { "Vin", "R2" };
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      debugE("Can not set VoltageDividerR1: configuration: missing json field %s\n", str.c_str());
      return false;
    }
  }

  Vin = config["Vin"];
  R2 = config["R2"];

  return true;

}
