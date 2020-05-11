#include "voltagedividerR2.h"

VoltageDividerR2::VoltageDividerR2(float R1, float Vin, String config_path) :
   SymmetricTransform<float>(config_path ), R1{R1}, Vin{Vin} {
     className = "VoltageDividerR2";
     load_configuration();
}

void VoltageDividerR2::set_input(float Vout, uint8_t ignored) {
    output = (Vout * R1) / (Vin - Vout);
    notify();
}


JsonObject& VoltageDividerR2::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["Vin"] = Vin;
  root["R1"] = R1;
  return root;
}

static const char SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "Vin": { "title": "Voltage in", "type": "number" },
        "R1": { "title": "Resistance (ohms) of R1", "type": "number" }
    }
  })";

String VoltageDividerR2::get_config_schema() {
  return FPSTR(SCHEMA);
}

bool VoltageDividerR2::set_configuration(const JsonObject& config) {

  String expected[] = { "Vin", "R1" };
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      debugE("Can not set VoltageDividerR2: configuration: missing json field %s\n", str.c_str());
      return false;
    }
  }

  Vin = config["Vin"];
  R1 = config["R1"];

  return true;

}
