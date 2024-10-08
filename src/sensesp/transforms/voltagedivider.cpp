#include "voltagedivider.h"

namespace sensesp {

VoltageDividerR1::VoltageDividerR1(float R2, float Vin,
                                   const String& config_path)
    : SymmetricTransform<float>(config_path), R2_{R2}, Vin_{Vin} {
  load();
}

void VoltageDividerR1::set(const float& Vout) {
  this->emit((Vin_ - Vout) * R2_ / Vout);
}

bool VoltageDividerR1::to_json(JsonObject& root) {
  root["Vin"] = Vin_;
  root["R2"] = R2_;
  return true;
}

bool VoltageDividerR1::from_json(const JsonObject& config) {
  String const expected[] = {"Vin", "R2"};
  for (auto str : expected) {
    if (!config[str].is<JsonVariant>()) {
      ESP_LOGE(
          __FILENAME__,
          "Cannot set VoltageDividerR1: configuration: missing json field %s\n",
          str.c_str());
      return false;
    }
  }

  Vin_ = config["Vin"];
  R2_ = config["R2"];

  return true;
}

const String ConfigSchema(const VoltageDividerR1& obj) {
  return R"({"type":"object","properties":{"Vin":{"title":"Voltage in","type":"number"},"R2":{"title":"Resistance (ohms) of R2","type":"number"}}})";
}

VoltageDividerR2::VoltageDividerR2(float R1, float Vin,
                                   const String& config_path)
    : SymmetricTransform<float>(config_path), R1_{R1}, Vin_{Vin} {
  load();
}

void VoltageDividerR2::set(const float& Vout) {
  this->emit((Vout * R1_) / (Vin_ - Vout));
}

bool VoltageDividerR2::to_json(JsonObject& root) {
  root["Vin"] = Vin_;
  root["R1"] = R1_;
  return true;
}

bool VoltageDividerR2::from_json(const JsonObject& config) {
  String const expected[] = {"Vin", "R1"};
  for (auto str : expected) {
    if (!config[str].is<JsonVariant>()) {
      ESP_LOGE(
          __FILENAME__,
          "Cannot set VoltageDividerR2: configuration: missing json field %s\n",
          str.c_str());
      return false;
    }
  }

  Vin_ = config["Vin"];
  R1_ = config["R1"];

  return true;
}

const String ConfigSchema(const VoltageDividerR2& obj) {
  return R"({"type":"object","properties":{"Vin":{"title":"Voltage in","type":"number"},"R1":{"title":"Resistance (ohms) of R1","type":"number"}}})";
}

}  // namespace sensesp
