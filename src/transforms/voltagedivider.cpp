#include "voltagedivider.h"

VoltageDividerR1::VoltageDividerR1(float R2, float Vin, String config_path)
    : SymmetricTransform<float>(config_path), R2_{R2}, Vin_{Vin} {
  load_configuration();
}

void VoltageDividerR1::set_input(float Vout, uint8_t ignored) {
  this->emit((Vin_ - Vout) * R2_ / Vout);
}

void VoltageDividerR1::get_configuration(JsonObject& root) {
  root["Vin"] = Vin_;
  root["R2"] = R2_;
}

static const char SCHEMA_R1[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "Vin": { "title": "Voltage in", "type": "number" },
        "R2": { "title": "Resistance (ohms) of R2", "type": "number" }
    }
  })";

String VoltageDividerR1::get_config_schema() { return FPSTR(SCHEMA_R1); }

bool VoltageDividerR1::set_configuration(const JsonObject& config) {
  String expected[] = {"Vin", "R2"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      debugE(
          "Cannot set VoltageDividerR1: configuration: missing json field %s\n",
          str.c_str());
      return false;
    }
  }

  Vin_ = config["Vin"];
  R2_ = config["R2"];

  return true;
}

VoltageDividerR2::VoltageDividerR2(float R1, float Vin, String config_path)
    : SymmetricTransform<float>(config_path), R1_{R1}, Vin_{Vin} {
  load_configuration();
}

void VoltageDividerR2::set_input(float Vout, uint8_t ignored) {
  this->emit((Vout * R1_) / (Vin_ - Vout));
}

void VoltageDividerR2::get_configuration(JsonObject& root) {
  root["Vin"] = Vin_;
  root["R1"] = R1_;
}

static const char SCHEMA_R2[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "Vin": { "title": "Voltage in", "type": "number" },
        "R1": { "title": "Resistance (ohms) of R1", "type": "number" }
    }
  })";

String VoltageDividerR2::get_config_schema() { return FPSTR(SCHEMA_R2); }

bool VoltageDividerR2::set_configuration(const JsonObject& config) {
  String expected[] = {"Vin", "R1"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      debugE(
          "Cannot set VoltageDividerR2: configuration: missing json field %s\n",
          str.c_str());
      return false;
    }
  }

  Vin_ = config["Vin"];
  R1_ = config["R1"];

  return true;
}
