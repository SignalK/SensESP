#ifndef SENSESP_UI_UI_CONTROLS_H
#define SENSESP_UI_UI_CONTROLS_H

#include "sensesp.h"

#include "sensesp/ui/config_item.h"

namespace sensesp {

class StringConfig : public FileSystemSaveable {
 public:
  StringConfig(String& value, String& config_path)
      : FileSystemSaveable(config_path), value_(value) {
    load();
  }

  virtual bool to_json(JsonObject& doc) override;
  virtual bool from_json(const JsonObject& config) override;

  String get_value() { return value_; }

 protected:
  String value_;
  String title_ = "Value";
};

inline const String ConfigSchema(const StringConfig& obj) {
  return R"({"type":"object","properties":{"value":{"title":"Value","type":"string"}}})";
}

class NumberConfig : public FileSystemSaveable {
 public:
  NumberConfig(float& value, String& config_path)
      : FileSystemSaveable(config_path), value_(value) {
    load();
  }

  virtual bool to_json(JsonObject& doc) override;
  virtual bool from_json(const JsonObject& config) override;
  float get_value() { return value_; }

 protected:
  float value_;
  String title_ = "Value";

  friend const String ConfigSchema(const NumberConfig& obj);
};

inline const String ConfigSchema(const NumberConfig& obj) {
  String schema = R"({"type":"object","properties":{"value":{"title":"{{title}}","type":"number"}}})";
  schema.replace("{{title}}", obj.title_);
  return schema.c_str();
}

class CheckboxConfig : public FileSystemSaveable {
 public:
  CheckboxConfig(bool value, String title, String config_path)
      : FileSystemSaveable(config_path), value_(value), title_(title) {
    load();
  }

  virtual bool to_json(JsonObject& doc) override;
  virtual bool from_json(const JsonObject& config) override;

  bool get_value() { return value_; }

 protected:
  bool value_ = false;
  String title_ = "Enable";

  friend const String ConfigSchema(const CheckboxConfig& obj);
};

inline const String ConfigSchema(const CheckboxConfig& obj) {
  String schema = R"({"type":"object","properties":{"value":{"title":"{{title}}","type":"boolean"}}})";
  schema.replace("{{title}}", obj.title_);
  return schema.c_str();
}

enum class SelectType {
  kUndefined,
  kCheckbox,
  kSelect,
};

class SelectConfig : public FileSystemSaveable {
 public:
  SelectConfig(String value, String title, String config_path,
               std::vector<String> options, SelectType format)
      : FileSystemSaveable(config_path),
        value_(value),
        title_(title),
        options_(options),
        format_(format) {
    load();
  }

  virtual bool to_json(JsonObject& doc) override;
  virtual bool from_json(const JsonObject& config) override;

  String get_value() { return value_; }

 protected:
  String value_;
  String title_ = "Value";
  std::vector<String> options_;
  SelectType format_ = SelectType::kUndefined;

  friend const String ConfigSchema(const SelectConfig& obj);
};

inline const String ConfigSchema(const SelectConfig& obj) {
  String schema = R"({"type":"object","properties":{"value":{"title":"<<title>>","type":"array","format":"<<format>>","uniqueItems":true,"items":{"type":"string","enum":[<<options>>]}}}})";
  schema.replace("<<title>>", obj.title_);
  String options;
  for (size_t i = 0; i < obj.options_.size(); i++) {
    options += "\"" + obj.options_[i] + "\"";
    if (i < obj.options_.size() - 1) {
      options += ",";
    }
  }
  schema.replace("<<options>>", options);
  String format;
  if (obj.format_ == SelectType::kUndefined) {
    format = "undefined";
  } else if (obj.format_ == SelectType::kCheckbox) {
    format = "checkbox";
  } else if (obj.format_ == SelectType::kSelect) {
    format = "select";
  }
  schema.replace("<<format>>", format);
  return schema.c_str();
}

}  // namespace sensesp

#endif  // USERS_MAIRAS_SRC_SIGNALK_SENSESP_SRC_SENSESP_UI_UI_CONTROLS_H
