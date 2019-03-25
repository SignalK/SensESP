#include "transform.h"

#include "ArduinoJson.h"

#include "sensesp.h"

// Transform

std::set<Transform*> Transform::transforms;

Transform::Transform(String sk_path, String id, String schema)
    : Configurable{id, schema}, sk_path{sk_path} {
  transforms.insert(this);
}

