#include "constant_sensor.h"

namespace sensesp {

// ..........................................
// base class for constant sensors
// ..........................................

ConstantSensor::ConstantSensor(int send_interval, String config_path = "")
    : SensorT<float>(config_path), send_interval_{send_interval} {
  load_configuration();
}

void ConstantSensor::start() {
  ReactESP::app->onRepeat(send_interval_ * 1000, [this]() { this->emit(value_); });
}

void ConstantSensor::update() { this->emit(value_); }

String ConstantSensor::get_config_schema() {
  return FPSTR(SCHEMA_CONSTANT_SENSOR);
}

void ConstantSensor::get_configuration(JsonObject &root) {
  root["value"] = value_;
}

bool ConstantSensor::set_configuration(const JsonObject &config) {
  String expected[] = {"value"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  value_ = config["value"];
  return true;
}

// ..........................................
// constant sensors
// ..........................................

//
// constant float
ConstantFloat::ConstantFloat(int send_delay, String config_path = "")
    : ConstantSensor(send_delay, config_path) {}
void ConstantFloat::setValue(float value) { value_ = value; }
float ConstantFloat::getValue() { return value_; }

// constant int
ConstantInt::ConstantInt(int send_delay, String config_path = "")
    : ConstantSensor(send_delay, config_path) {}
void ConstantInt::setValue(int value) { value_ = value; }
int ConstantInt::getValue() { return value_; }

//
// constant String
ConstantString::ConstantString(int send_delay, String config_path = "")
    : ConstantSensor(send_delay, config_path) {}
void ConstantString::setValue(String value) { value_ = value; }
String ConstantString::getValue() { return value_; }

}  // namespace sensesp