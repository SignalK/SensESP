#ifndef _signalk_output_H_
#define _signalk_output_H_

#include "signalk/signalk_emitter.h"
#include "transforms/transform.h"

static const char SIGNALKOUTPUT_SCHEMA[] PROGMEM = R"({
      "type": "object",
      "properties": {
          "sk_path": { "title": "SignalK Path", "type": "string" }
      }
  })";

// SKOutput is a specialized transform whose primary purpose is
// to output SignalK data on the SignalK network.
template <typename T>
class SKOutput : public SKEmitter,
                      public SymmetricTransform<T> {
 public:
  SKOutput() : SKOutput("") {}

  SKOutput(String sk_path, String config_path="")
    : SKEmitter(sk_path), SymmetricTransform<T>(config_path) {
    Enable::className = "SKOutput";
    Enable::setPriority(-5);
  }


  virtual void set_input(T newValue, uint8_t inputChannel = 0) override {
    ValueProducer<T>::output = newValue;
    this->notify();
  }


  virtual String as_signalK() override {
    DynamicJsonBuffer jsonBuffer;
    String json;
    JsonObject& root = jsonBuffer.createObject();
    root.set("path", this->get_sk_path());
    root.set("value", ValueProducer<T>::output);
    root.printTo(json);
    return json;
  }

  virtual JsonObject& get_configuration(JsonBuffer& buf) override {
    JsonObject& root = buf.createObject();
    root["sk_path"] = this->get_sk_path();
    return root;
  }

  String get_config_schema() override {
    return FPSTR(SIGNALKOUTPUT_SCHEMA);
  }

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
