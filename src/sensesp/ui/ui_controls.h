#ifndef SENSESP_SRC_UI_CONTROLS_H_
#define SENSESP_SRC_UI_CONTROLS_H_

#include "sensesp.h"

#include "sensesp/system/configurable.h"

namespace sensesp {

class StringConfig : public Configurable {
 public:
  StringConfig(String& value, String& config_path)
      : value_(value), Configurable(config_path) {
    load_configuration();
  }

  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;

  String get_value() { return value_; }

 protected:
  String value_;
  String title_ = "Value";
};

class NumberConfig : public Configurable {
 public:
  NumberConfig(float& value, String& config_path)
      : value_(value), Configurable(config_path) {
    load_configuration();
  }

  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;

  float get_value() { return value_; }

 protected:
  float value_;
  String title_ = "Value";
};

class CheckboxConfig : public Configurable {
 public:
  CheckboxConfig(bool value, String title, String config_path)
      : value_(value), title_(title), Configurable(config_path) {
    load_configuration();
  }

  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;

  bool get_value() { return value_; }

 protected:
  bool value_ = false;
  String title_ = "Enable";
};

enum class SelectType {
  kUndefined,
  kCheckbox,
  kSelect,
};

class SelectConfig : public Configurable {
 public:
  SelectConfig(String value, String title, String config_path,
               std::vector<String> options, SelectType format)
      : value_(value),
        title_(title),
        options_(options),
        format_(format),
        Configurable(config_path) {
    load_configuration();
  }

  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;

  String get_value() { return value_; }

 protected:
  String value_;
  String title_ = "Value";
  std::vector<String> options_;
  SelectType format_ = SelectType::kUndefined;
};

}  // namespace sensesp

#endif  // SENSESP_SRC_UI_CONTROLS_H_
