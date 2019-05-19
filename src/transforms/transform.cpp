#include "transform.h"

#include "ArduinoJson.h"

#include "sensesp.h"

// Transform

std::set<TransformBase*> TransformBase::transforms;

TransformBase::TransformBase(String sk_path, String id, String schema) :
    Configurable{id, schema}, SignalKSource(sk_path), MustEnable(5) {
  transforms.insert(this);
}
