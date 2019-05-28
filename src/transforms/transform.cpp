#include "transform.h"

#include "ArduinoJson.h"

#include "sensesp.h"

// Transform

std::set<TransformBase*> TransformBase::transforms;

TransformBase::TransformBase(String sk_path, String config_path) :
    Configurable{config_path}, SignalKSource(sk_path), Enable(5) {
  transforms.insert(this);
}
