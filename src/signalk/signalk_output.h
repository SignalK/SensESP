#ifndef _signalk_output_H_
#define _signalk_output_H_

#include "signalk/signalk_emitter.h"
#include "transforms/transform.h"

static const char SIGNALKOUTPUT_SCHEMA[] PROGMEM = R"({
      "type": "object",
      "properties": {
          "sk_path": { "title": "Signal K Path", "type": "string" }
      }
  })";

// SKOutput is a specialized transform whose primary purpose is
// to output Signal K data on the Signal K network.
template <typename T>
class SKOutput : public SKEmitter, public SymmetricTransform<T> {
 public:
  SKOutput() : SKOutput("") {}

  SKOutput(String sk_path, String config_path = "")
      : SKEmitter(sk_path), SymmetricTransform<T>(config_path) {
    Enable::set_priority(-5);
  }

  virtual void set_input(T new_value, uint8_t input_channel = 0) override {
    this->ValueProducer<T>::emit(new_value);
  }

  virtual String as_signalk() override {
    DynamicJsonDocument json_doc(1024);
    String json;
    json_doc["path"] = this->get_sk_path();
    json_doc["value"] = ValueProducer<T>::output;
    serializeJson(json_doc, json);
    return json;
  }

  virtual void get_configuration(JsonObject& root) override {
    root["sk_path"] = this->get_sk_path();
  }

  String get_config_schema() override { return FPSTR(SIGNALKOUTPUT_SCHEMA); }

  virtual bool set_configuration(const JsonObject& config) override {
    if (!config.containsKey("sk_path")) {
      return false;
    }
    this->set_sk_path(config["sk_path"].as<String>());
    return true;
  }
};

typedef SKOutput<float> SKOutputNumber;
typedef SKOutput<int> SKOutputInt;
typedef SKOutput<bool> SKOutputBool;
typedef SKOutput<String> SKOutputString;

#endif
