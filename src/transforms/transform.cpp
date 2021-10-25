#include "transform.h"

#include "ArduinoJson.h"
#include "sensesp.h"

// Transform

std::set<TransformBase*> TransformBase::transforms_;

TransformBase::TransformBase(String config_path)
    : Configurable{config_path}, Startable(5) {
  transforms_.insert(this);
}
