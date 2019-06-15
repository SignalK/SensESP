#ifndef _sensor_H_
#define _sensor_H_

#include <set>

#include "system/configurable.h"
#include "system/observable.h"
#include "system/valueproducer.h"
#include "system/enable.h"


///////////////////
// Sensors represent raw hardware thingies.

class Sensor : virtual public Observable, public Configurable, public Enable {
  public:
    Sensor(String config_path="");

  static const std::set<Sensor*>& get_sensors() {
    return sensors;
  }

  private:
    static std::set<Sensor*> sensors;
};



class NumericSensor : public Sensor, public NumericProducer {

    public:
        NumericSensor(String config_path="");

};


class IntegerSensor : public Sensor, public IntegerProducer {

    public:
        IntegerSensor(String config_path="");

};


class StringSensor : public Sensor, public StringProducer {

    public:
        StringSensor(String config_path="");

};

#endif