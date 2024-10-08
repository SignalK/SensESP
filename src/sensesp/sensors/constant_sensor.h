#ifndef SENSESP_SENSORS_CONSTANT_SENSOR_H_
#define SENSESP_SENSORS_CONSTANT_SENSOR_H_

#include "Arduino.h"
#include "sensesp/ui/config_item.h"
#include "sensesp/sensors/sensor.h"
#include "sensesp_base_app.h"

namespace sensesp {

/**
 * @brief ConstantSensor is the base class for virtual sensors that periodically
 * emit a constant value.
 *
 * The virtual sensors are typically used to send a constant value to SignalK.
 * Typical example is the capacity of a tank or the length of an anchor rode;
 *
 * The sensors can be connected to all the standard SKOutput consumers.
 *
 * The sensor value and send delay can be configured via the web ui.
 *
 * The sensor has a get_value() and a set_value() method to interact with the
 * sensor from normal code.
 *
 * Example: send the water tank capacity every 30 seconds to SignalK:
 *
 *    int send_delay = 30;
 *    const char* config_path = "/tanks.water_capacity";
 *    auto *capacity = new ConstantFloatSensor(send_delay,config_path);
 *    capacity->connect_to(new SKOutputFloat(sk_path, sk_config_path,
 * sk_metadata));
 *
 * To set or get the the value of the virtual sensor for use in your code:
 *
 *    capacity->set_value(value);
 *
 *    value = capacity->get_value();
 *
 * @param send_interval[in] Time interval in seconds between consecutive
 * emissions of the sensor value.
 *
 * @param[in] config_path Configuration path for the sensor.
 */

/**
 * @brief Base class for constant value sensors.
 */
template <class T>
class ConstantSensor : public Sensor<T> {
 public:
  ConstantSensor(T value, int send_interval = 30, String config_path = "")
      : Sensor<T>(config_path), value_{value}, send_interval_{send_interval} {
    this->load();

    // Emit the initial value once to set the output
    event_loop()->onDelay(0, [this]() { this->emit(value_); });
    // Then, emit the value at the specified interval
    event_loop()->onRepeat(send_interval_ * 1000,
                           [this]() { this->emit(value_); });
  }

  void set(T value) { value_ = value; }

  virtual bool to_json(JsonObject& doc) override {
    doc["value"] = value_;
    return true;
  }
  virtual bool from_json(const JsonObject& config) override {
    // Neither of the configuration parameters are mandatory
    if (config["value"].is<T>()) {
      value_ = config["value"].as<T>();
    }
    return true;
  }

 protected:
  void update() { this->emit(value_); }

  T value_;
  int send_interval_;  // seconds
};

template <typename T>
const String ConfigSchemaSensorType(const T& obj) {
  return "";
}
template <>
const String ConfigSchemaSensorType<int>(const int& obj) {
  return "integer";
}
template <>
const String ConfigSchemaSensorType<float>(const float& obj) {
  return "number";
}
template <>
const String ConfigSchemaSensorType<String>(const String& obj) {
  return "string";
}
template <>
const String ConfigSchemaSensorType<bool>(const bool& obj) {
  return "boolean";
}

template <typename T>
const String ConfigSchema(const ConstantSensor<T>& obj) {
  String schema = R"###({"type":"object","properties":{"value":{"title":"Constant Value","type":"%TYPE%","description":"Constant value"}}    })###";
  ;
  schema.replace("%TYPE%", ConfigSchemaSensorType(obj));
  return schema.c_str();
}

// ..........................................
//  constant value sensors
// ..........................................

typedef ConstantSensor<float> FloatConstantSensor;
typedef ConstantSensor<int> IntConstantSensor;
typedef ConstantSensor<bool> BoolConstantSensor;
typedef ConstantSensor<String> StringConstantSensor;

}  // namespace sensesp

#endif
