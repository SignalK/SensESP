#include "constant_sensor.h"

namespace sensesp {

// ..........................................
// base class for constant sensors
// ..........................................
template <typename T>
ConstantSensor<T>::ConstantSensor(int send_interval, String config_path)
    : SensorT<T>(config_path), send_interval_{send_interval} {
  this->load_configuration();
}

template <typename T>
void ConstantSensor<T>::start() {
  ReactESP::app->onRepeat(send_interval_ * 1000,
                          [this]() { this->emit(value_); });
}

template <typename T>
void ConstantSensor<T>::update() {
  this->emit(value_);
}

template <typename T>
String ConstantSensor<T>::get_config_schema() {
  return FPSTR(SCHEMA_CONSTANT_SENSOR);
}
template <typename T>
void ConstantSensor<T>::get_configuration(JsonObject &root) {
  root["value"] = value_;
}

template <typename T>
bool ConstantSensor<T>::set_configuration(const JsonObject &config) {
  String expected[] = {"value"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  value_ = config["value"];
  return true;
}

template <typename T>
void ConstantSensor<T>::setValue(T value) {
  value_ = value;
}

template <typename T>
T ConstantSensor<T>::getValue() {
  return value_;
}

}  // namespace sensesp
