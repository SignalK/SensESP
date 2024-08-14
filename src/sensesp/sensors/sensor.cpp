#include "sensor.h"

namespace sensesp {

std::set<SensorConfig*> SensorConfig::sensors_;

SensorConfig::SensorConfig(String config_path) : Configurable{config_path} {
  sensors_.insert(this);
}

}  // namespace sensesp
