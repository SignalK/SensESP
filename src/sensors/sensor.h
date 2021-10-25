#ifndef _sensor_H_
#define _sensor_H_

#include <set>

#include "system/configurable.h"
#include "system/observable.h"
#include "system/valueproducer.h"
#include "system/startable.h"


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
class Sensor : virtual public Observable, public Configurable, public Startable {
  public:
    Sensor(String config_path = "");

  static const std::set<Sensor*>& get_sensors() {
    return sensors_;
  }

  private:
    static std::set<Sensor*> sensors_;
};


/**
 * @brief A Sensor whose output is a float.
 **/ 
class FloatSensor : public Sensor, public FloatProducer {

    public:
        FloatSensor(String config_path = "");

};

/**
 * @brief A Sensor whose output is an integer.
 **/
class IntSensor : public Sensor, public IntProducer {

    public:
        IntSensor(String config_path = "");

};

/**
 * @brief A Sensor whose output is a String.
 **/
class StringSensor : public Sensor, public StringProducer {

    public:
        StringSensor(String config_path = "");
};

#endif