#include "ui_controls.h"

#include <Arduino.h>

namespace sensesp {

bool StringConfig::to_json(JsonObject& doc) {
  doc["value"] = value_;
  return true;
}

bool StringConfig::from_json(const JsonObject& config) {
  if (!config["value"].is<String>()) {
    return false;
  }

  value_ = config["value"].as<String>();

  return true;
}

bool NumberConfig::to_json(JsonObject& doc) {
  doc["value"] = value_;
  return true;
}

bool NumberConfig::from_json(const JsonObject& config) {
  if (!config["value"].is<float>()) {
    return false;
  }

  value_ = config["value"];

  return true;
}

bool CheckboxConfig::to_json(JsonObject& doc) {
  doc["value"] = value_;
  return true;
}

bool CheckboxConfig::from_json(const JsonObject& config) {
  if (!config["value"].is<bool>()) {
    return false;
  }

  value_ = config["value"];

  return true;
}

bool SelectConfig::to_json(JsonObject& doc) {
  doc["value"] = value_;
  return true;
}

bool SelectConfig::from_json(const JsonObject& config) {
  if (!config["value"].is<String>()) {
    return false;
  }

  value_ = config["value"].as<String>();

  return true;
}

}  // namespace sensesp
