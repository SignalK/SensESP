#ifndef CONSTANT_SENSOR_H_
#define CONSTANT_SENSOR_H_

#include "Arduino.h"
#include "sensesp/sensors/sensor.h"

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

static const char SCHEMA_CONSTANT_SENSOR[] PROGMEM = R"###({
        "type": "object",
        "properties": {
            "value": { "title": "Constant Value", "type": "number", "description": "Constant value" }
        }
    })###";

/**
 * @brief Base class for constant value sensors.
 */
template <class T>
class ConstantSensor : public SensorT<T> {
 public:
  ConstantSensor(T value, int send_interval = 30, String config_path = "")
      : SensorT<T>(config_path), value_{value}, send_interval_{send_interval} {
    this->load_configuration();

    ReactESP::app->onRepeat(send_interval_ * 1000,
                            [this]() { this->emit(value_); });
  }

  void set_value(T value) { value_ = value; }
  T get_value() { return value_; }

 protected:
  virtual void get_configuration(JsonObject &doc) override {
    doc["value"] = value_;
    doc["interval"] = send_interval_;
  }
  virtual bool set_configuration(const JsonObject &config) override {
    // Neither of the configuration parameters are mandatory
    if (config.containsKey("value")) {
      value_ = config["value"];
    }
    if (config.containsKey("interval")) {
      send_interval_ = config["interval"];
    }
    return true;
  }
  virtual String get_config_schema() override {
    return FPSTR(SCHEMA_CONSTANT_SENSOR);
  }
  void update() { this->emit(value_); }

  T value_;
  int send_interval_;  // seconds
};

// ..........................................
//  constant value sensors
// ..........................................

typedef ConstantSensor<float> FloatConstantSensor;
typedef ConstantSensor<int> IntConstantSensor;
typedef ConstantSensor<bool> BoolConstantSensor;
typedef ConstantSensor<String> StringConstantSensor;

}  // namespace sensesp

#endif
