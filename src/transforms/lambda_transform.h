#ifndef _lambda_transform_H_
#define _lambda_transform_H_

#include "transform.h"

static const char kLambdaTransformSchema[] PROGMEM = R"({
    "type": "object",
    "properties": {
      "value": { "title": "Last value", "type": "number", "readOnly": true }
    }
  })";

template <typename I, typename O>
class LambdaTransform : public Transform<I, O> {
 private:
  O (*function)(I input);

 public:
  LambdaTransform(O (*function)(I input), String config_path = "")
      : Transform<I, O>(config_path) {
    this->function = function;
  }

  void set_input(I input, uint8_t input_channel = 0) override {
    this->output = (*function)(input);
  }

  void get_configuration(JsonObject& doc) override {}
  bool set_configuration(const JsonObject& config) override {}
  String get_config_schema() override { return FPSTR(kLambdaTransformSchema); }
};

#endif
