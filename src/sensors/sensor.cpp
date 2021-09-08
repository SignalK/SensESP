#include "sensor.h"

std::set<Sensor*> Sensor::sensors_;

Sensor::Sensor(String config_path) : Configurable{config_path}, Enableable(10) {
  sensors_.insert(this);
}

FloatSensor::FloatSensor(String config_path)
    : Sensor(config_path), FloatProducer(){};

IntSensor::IntSensor(String config_path)
    : Sensor(config_path), IntProducer(){};

StringSensor::StringSensor(String config_path)
    : Sensor(config_path), StringProducer(){};
