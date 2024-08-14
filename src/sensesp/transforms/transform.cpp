#include "transform.h"

#include "ArduinoJson.h"
#include "sensesp.h"

namespace sensesp {

// Transform

std::set<TransformBase*> TransformBase::transforms_;

TransformBase::TransformBase(String config_path)
    : Configurable{config_path} {
  transforms_.insert(this);
}

}  // namespace sensesp
