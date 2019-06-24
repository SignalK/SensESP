#include "sensor.h"

std::set<Sensor*> Sensor::sensors;

Sensor::Sensor(String config_path) : Configurable{config_path}, Enable(10) {
  className = "Sensor";
  sensors.insert(this);
}



NumericSensor::NumericSensor(String config_path) :
   Sensor(config_path), NumericProducer() {
      className = "NumericSensor";

};


IntegerSensor::IntegerSensor(String config_path) :
   Sensor(config_path), IntegerProducer() {
      className = "IntegerSensor";

};


StringSensor::StringSensor(String config_path) :
   Sensor(config_path), StringProducer() {
      className = "StringSensor";

};
