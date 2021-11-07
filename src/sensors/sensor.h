#ifndef _sensor_H_
#define _sensor_H_

#include <set>

#include "system/configurable.h"
#include "system/observable.h"
#include "system/startable.h"
#include "system/valueproducer.h"

namespace sensesp {

/**
 * @brief The base class for all sensors. Used only as a base class - never
 * instantiated directly in a project.
 *
 * In SensESP, a sensor is something that takes a measurement outside of the
 * SensESP / SignalK environment and brings it into SensESP. Look at the list
 * of files in https://github.com/SignalK/SensESP/tree/master/src/sensors for
 * all the already-defined sensors.
 *
 * @param config_path The path that defines the place in the hierarchy of
 * variables, in the Config UI, that are configurable at run-time. If a class
 * has no configurable variables, or if you simply don't want any particular
 * variable to be configurable at run-time in your project, don't provide a
 * config_path when you construct the class.
 */
class Sensor : virtual public Observable,
               public Configurable,
               public Startable {
 public:
  Sensor(String config_path = "");

  static const std::set<Sensor*>& get_sensors() { return sensors_; }

 private:
  static std::set<Sensor*> sensors_;
};

/**
 * @brief Sensor template class for any sensor producing actual values.
 **/
template <typename T>
class SensorT : public Sensor, public ValueProducer<T> {
 public:
  SensorT<T>(String config_path = "")
      : Sensor(config_path), ValueProducer<T>() {}
};

typedef SensorT<float> FloatSensor;
typedef SensorT<int> IntSensor;
typedef SensorT<String> StringSensor;

}  // namespace sensesp

#endif
