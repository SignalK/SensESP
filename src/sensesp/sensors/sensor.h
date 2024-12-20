#ifndef SENSESP_SENSORS_SENSOR_H_
#define SENSESP_SENSORS_SENSOR_H_

#include <Arduino.h>
#include <set>

#include "sensesp/system/observable.h"
#include "sensesp/system/saveable.h"
#include "sensesp/system/valueproducer.h"
#include "sensesp_base_app.h"

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
class SensorConfig : virtual public Observable, public FileSystemSaveable {
 public:
  SensorConfig(const String& config_path) : FileSystemSaveable(config_path) {}

 private:
};

/**
 * @brief Sensor template class for any sensor producing actual values.
 *
 **/
template <typename T>
class Sensor : public SensorConfig, virtual public ValueProducer<T> {
 public:
  Sensor(String config_path) : SensorConfig(config_path), ValueProducer<T>() {}
};

typedef Sensor<float> FloatSensor;
typedef Sensor<int> IntSensor;
typedef Sensor<bool> BoolSensor;
typedef Sensor<String> StringSensor;

template <class T>
class RepeatSensor : public Sensor<T> {
 public:
  /**
   * @brief Construct a new RepeatSensor object.
   *
   * RepeatSensor is a sensor that calls a callback function at given intervals
   * and produces the value returned by the callback function. It can be used
   * to wrap any generic Arduino sensor library into a SensESP sensor.
   *
   * @param repeat_interval_ms The repeating interval, in milliseconds.
   * @param callback A callback function that returns the value the sensor will
   * produce.
   * @tparam T The type of the value returned by the callback function.
   */
  RepeatSensor(unsigned int repeat_interval_ms, std::function<T()> callback)
      : Sensor<T>(""),
        repeat_interval_ms_(repeat_interval_ms),
        returning_callback_(callback) {
    event_loop()->onRepeat(repeat_interval_ms_, [this]() {
      this->emit(this->returning_callback_());
    });
  }

  /**
   * @brief Construct a new RepeatSensor object (supporting asynchronous
   * callbacks).
   *
   * RepeatSensor is a sensor that calls a callback function at given intervals
   * and produces the value returned by the callback function. It can be used
   * to wrap any generic Arduino sensor library into a SensESP sensor.
   *
   * @param repeat_interval_ms The repeating interval, in milliseconds.
   * @param callback A callback function that requires RepeatSensor<T>::emit()
   *   to be called when output becomes available.
   * @tparam T The type of the value returned by the callback function.
   */
  RepeatSensor(unsigned int repeat_interval_ms,
               std::function<void(RepeatSensor<T>*)> callback)
      : Sensor<T>(""),
        repeat_interval_ms_(repeat_interval_ms),
        emitting_callback_(callback) {
    event_loop()->onRepeat(repeat_interval_ms_,
                           [this]() { emitting_callback_(this); });
  }

 protected:
  unsigned int repeat_interval_ms_;
  std::function<T()> returning_callback_ = nullptr;
  std::function<void(RepeatSensor<T>*)> emitting_callback_ = nullptr;
};

}  // namespace sensesp

#endif
