#include "ui_controls.h"

#include <Arduino.h>

namespace sensesp {

static const char kStringConfigSchemaTemplate[] = R"({
    "type": "object",
    "properties": {
        "value": { "title": "{{title}}", "type": "string" }
    }
  })";

String StringConfig::get_config_schema() {
  String schema = kStringConfigSchemaTemplate;
  schema.replace("{{title}}", title_);
  return schema;
}

void StringConfig::get_configuration(JsonObject& doc) { doc["value"] = value_; }

bool StringConfig::set_configuration(const JsonObject& config) {
  if (!config.containsKey("value")) {
    return false;
  }

  value_ = config["value"].as<String>();

  return true;
}

static const char kNumberConfigSchemaTemplate[] = R"({
    "type": "object",
    "properties": {
        "value": { "title": "{{title}}", "type": "number" }
    }
  })";

String NumberConfig::get_config_schema() {
  String schema = kNumberConfigSchemaTemplate;
  schema.replace("{{title}}", title_);
  return schema;
}

void NumberConfig::get_configuration(JsonObject& doc) { doc["value"] = value_; }

bool NumberConfig::set_configuration(const JsonObject& config) {
  if (!config.containsKey("value")) {
    return false;
  }

  value_ = config["value"];

  return true;
}

static const char kCheckboxConfigSchemaTemplate[] = R"({
    "type": "object",
    "properties": {
        "value": { "title": "{{title}}", "type": "boolean" }
    }
  })";

String CheckboxConfig::get_config_schema() {
  String schema = kCheckboxConfigSchemaTemplate;
  schema.replace("{{title}}", title_);
  return schema;
}

void CheckboxConfig::get_configuration(JsonObject& doc) {
  doc["value"] = value_;
}

bool CheckboxConfig::set_configuration(const JsonObject& config) {
  if (!config.containsKey("value")) {
    return false;
  }

  value_ = config["value"];

  return true;
}

static const char kSelectConfigSchemaTemplate[] = R"({
    "type": "object",
    "properties": {
        "value": {
            "title": "{{title}}",
            "type": "array",
            "format": "{format}",
            "uniqueItems": true,
            "items": {
                "type": "string", "enum": [{{options}}]
            }
        }
    }
  })";

String SelectConfig::get_config_schema() {
  String schema = kSelectConfigSchemaTemplate;
  schema.replace("{{title}}", title_);
  String options;
  for (size_t i = 0; i < options_.size(); i++) {
    options += "\"" + options_[i] + "\"";
    if (i < options_.size() - 1) {
      options += ",";
    }
  }
  schema.replace("{{options}}", options);
  String format;
  if (format_ == SelectType::kUndefined) {
    format = options_.size() >= 8 ? "select" : "checkbox";
  } else {
    format = format_ == SelectType::kCheckbox ? "checkbox" : "select";
  }
  schema.replace("{format}", options_.size() > 3 ? "checkbox" : "radio");
  return schema;
}

void SelectConfig::get_configuration(JsonObject& doc) { doc["value"] = value_; }

bool SelectConfig::set_configuration(const JsonObject& config) {
  if (!config.containsKey("value")) {
    return false;
  }

  value_ = config["value"].as<String>();

  return true;
}

}  // namespace sensesp
