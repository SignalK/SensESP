#include "sensor.h"

namespace sensesp {

std::set<Sensor*> Sensor::sensors_;

Sensor::Sensor(String config_path) : Configurable{config_path}, Startable(10) {
  sensors_.insert(this);
}

}  // namespace sensesp
