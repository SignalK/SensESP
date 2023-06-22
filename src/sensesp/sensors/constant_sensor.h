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
 * Typical example is the capacity of a tank.
 *
 * The sensors can be connected to all the standard SKOutput consumers.
 *
 * The sensor value and send delay can be configured via the web ui.
 *
 * The sensor has a getValue() and a setValue() method to interact with the
 * sensor from normal code;
 *
 * @param send_interval[in] Time interval in seconds between consecutive emissions of
 * the sensor value.
 *
 * @param[in] config_path Configuration path for the sensor.
 */

static const char SCHEMA_CONSTANT_SENSOR[] PROGMEM = R"###({
        "type": "object",
        "properties": {
            "value": { "title": "Constant Value", "type": "number", "description": "Constant value" }
        }
    })###";

// ..........................................
//  base class for constant value sensors
// ..........................................
class ConstantSensor : public SensorT<float> {
 public:
  ConstantSensor(int send_interval, String config_path);
  void start() override final;

 protected:
  virtual void get_configuration(JsonObject &doc) override;
  virtual bool set_configuration(const JsonObject &config) override;
  virtual String get_config_schema() override;
  void update();

 private:
  float value_;
  int send_interval_ = 30;  // seconds
};

// ..........................................
//  constant value sensors
// ..........................................

// float constant
class ConstantFloat : public ConstantSensor {
 public:
  ConstantFloat(int send_interval, String config_path);
  void setValue(float value);
  float getValue();

 private:
  float value_ = 0;
};

// float constant
class ConstantInt : public ConstantSensor {
 public:
  ConstantInt(int send_interval, String config_path);
  void setValue(int value);
  int getValue();

 private:
  int value_ = 0;
};

// String constant
class ConstantString : public ConstantSensor {
 public:
  ConstantString(int send_interval, String config_path);
  void setValue(String value);
  String getValue();

 private:
  String value_ = "";
};

}  // namespace sensesp

#endif