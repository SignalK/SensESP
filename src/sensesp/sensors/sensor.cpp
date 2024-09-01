#include "sensor.h"

#include <utility>

namespace sensesp {

std::set<SensorConfig*> SensorConfig::sensors_;

SensorConfig::SensorConfig(const String& config_path)
    : Configurable{config_path} {
  sensors_.insert(this);
}

}  // namespace sensesp
