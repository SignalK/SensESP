#include "sensor.h"

namespace sensesp {

std::set<Sensor*> Sensor::sensors_;

Sensor::Sensor(String config_path) : Configurable{config_path}, Startable(10) {
  sensors_.insert(this);
}

FloatSensor::FloatSensor(String config_path)
    : Sensor(config_path), FloatProducer(){};

IntSensor::IntSensor(String config_path) : Sensor(config_path), IntProducer(){};

StringSensor::StringSensor(String config_path)
    : Sensor(config_path), StringProducer(){};

}  // namespace sensesp
