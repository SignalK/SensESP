#ifndef _sensor_H_
#define _sensor_H_

#include <set>

#include "sensesp/system/configurable.h"
#include "sensesp/system/observable.h"
#include "sensesp/system/valueproducer.h"

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
class Sensor : virtual public Observable, public Configurable {
 public:
  Sensor(String config_path = "");

  static const std::set<Sensor*>& get_sensors() { return sensors_; }

 private:
  static std::set<Sensor*> sensors_;
};

/**
 * @brief Sensor template class for any sensor producing actual values.
 *
 **/
template <typename T>
class SensorT : public Sensor, public ValueProducer<T> {
 public:
  SensorT<T>(String config_path = "")
      : Sensor(config_path), ValueProducer<T>() {}
};

typedef SensorT<float> FloatSensor;
typedef SensorT<int> IntSensor;
typedef SensorT<bool> BoolSensor;
typedef SensorT<String> StringSensor;

template <class T>
class RepeatSensor : public SensorT<T> {
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
  RepeatSensor<T>(unsigned int repeat_interval_ms, std::function<T()> callback)
      : SensorT<T>(""),
        repeat_interval_ms_(repeat_interval_ms),
        returning_callback_(callback) {
    ReactESP::app->onRepeat(repeat_interval_ms_, [this]() {
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
  RepeatSensor<T>(unsigned int repeat_interval_ms,
                  std::function<void(RepeatSensor<T>*)> callback)
      : SensorT<T>(""),
        repeat_interval_ms_(repeat_interval_ms),
        emitting_callback_(callback) {
    ReactESP::app->onRepeat(repeat_interval_ms_,
                            [this]() { emitting_callback_(this); });
  }

 protected:
  unsigned int repeat_interval_ms_;
  std::function<T()> returning_callback_ = nullptr;
  std::function<void(RepeatSensor<T>*)> emitting_callback_ = nullptr;
};

}  // namespace sensesp

#endif
