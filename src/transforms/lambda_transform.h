#ifndef _lambda_transform_H_
#define _lambda_transform_H_

#include "transform.h"

static const char kLambdaTransformSchemaHead[] PROGMEM = R"({
    "type": "object",
    "properties": {
  )";

static const char kLambdaTransformSchemaRow[] PROGMEM = R"(
      "%s": { "title": "%s", "type": "%s", "readOnly": %s }
  )";

static const char kLambdaTransformSchemaTail[] PROGMEM = R"(
    }
  })";

template<typename T>
const char* get_schema_type_string(const T dummy) { return "string"; }

template<>
const char* get_schema_type_string(const int dummy) { return "number"; }

template<>
const char* get_schema_type_string(const float dummy) { return "number"; }

template<>
const char* get_schema_type_string(const String dummy) { return "string"; }

template<>
const char* get_schema_type_string(const bool dummy) { return "boolean"; }

template <typename I, typename O>
class LambdaTransform : public Transform<I, O> {
 public:
  LambdaTransform(O (*function)(I input), String config_path = "")
      : Transform<I, O>(config_path) {
    this->function = function;
  }

  void set_input(I input, uint8_t input_channel = 0) override {
    this->output = (*function)(input);
    Observable::notify();
  }

  void get_configuration(JsonObject& doc) override {
    doc["value"] = this->output;
  }

  bool set_configuration(const JsonObject& config) override { return true; }

  String get_config_schema() override {
    // FIXME: The heavy use of Strings puts a lot of faith in that class's
    // ability to not leak or corrupt memory. Would be better to receive
    // a character array pointer as an argument for writing the output to.
    String output = "";

    output.concat(FPSTR(kLambdaTransformSchemaHead));
    output.concat(format_schema_row(
        "value", "Last value", get_schema_type_string(this->output), true));
    output.concat(FPSTR(kLambdaTransformSchemaTail));

    return output;
  }

 private:
  O (*function)(I input);

  static String format_schema_row(const char key[], const char title[],
                                  const char type[], const bool read_only) {
    char row[100] = "";
    const char* read_only_str = read_only ? "true" : "false";

    sprintf_P(row, PSTR(kLambdaTransformSchemaRow), key, title, type,
              read_only_str);

    return String(row);
  }
};

template <>

#endif
