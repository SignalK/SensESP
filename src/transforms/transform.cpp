#include "transform.h"

#include "ArduinoJson.h"

#include "sensesp.h"

// Transform

std::set<TransformBase*> TransformBase::transforms;

TransformBase::TransformBase(String config_path) :
    Configurable{config_path}, Enable(5) {
  className = "TransformBase";    
  transforms.insert(this);
}

