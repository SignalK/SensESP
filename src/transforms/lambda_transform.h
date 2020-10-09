#ifndef _lambda_transform_H_
#define _lambda_transform_H_

#include "transform.h"

// template snippets for configuration schema

static const char kLambdaTransformSchemaHead[] PROGMEM = R"({
    "type": "object",
    "properties": {
  )";

static const char kLambdaTransformSchemaTail[] PROGMEM = R"(
    }
  })";

/**
 * Convert the variable type to a string representation of the type using
 * template specializations.
 **/
template <class T>
const char* get_schema_type_string(const T dummy) {
  return "string";
}

template <>
const char* get_schema_type_string(const int dummy) {
  return "number";
}

template <>
const char* get_schema_type_string(const float dummy) {
  return "number";
}

template <>
const char* get_schema_type_string(const String dummy) {
  return "string";
}

template <>
const char* get_schema_type_string(const bool dummy) {
  return "boolean";
}

/**
 * Configuration parameter information struct
 **/
struct ParamInfo {
  const char* key;
  const char* description;
};

// FIXME: This whole implementation should be written with variadic template
//        parameters once the SDK gets updated past GCC 5.2.

/**
 * @brief Construct a new transform based on a single function
 *
 * LambdaTransform provides an easy way of creating custom transformations
 * without having to create new Transform classes. You just create a transform
 * function and feed that to a LambdaTransform constructor together with
 * possible configuration parameters values and configuration parameter
 * metadata (key names and descriptions).
 *
 * @tparam I Transform function input value type
 * @tparam O Transform function output value type
 * @tparam T1 Transform function parameter 1 type
 * @tparam T2 Transform function parameter 2 type
 * @tparam T3 Transform function parameter 3 type
 * @tparam T4 Transform function parameter 4 type
 * @tparam T5 Transform function parameter 5 type
 * @tparam T6 Transform function parameter 6 type
 * */
template <class I, class O, class T1 = float, class T2 = float,
          class T3 = float, class T4 = float, class T5 = float,
          class T6 = float>
class LambdaTransform : public Transform<I, O> {
 public:
  LambdaTransform(O (*function)(I input), String config_path = "")
      : Transform<I, O>(config_path), function0{function}, num_params{0} {
    this->load_configuration();
  }

  LambdaTransform(O (*function)(I input), const ParamInfo* param_info,
                  String config_path = "")
      : Transform<I, O>(config_path),
        function0{function},
        num_params{0},
        param_info{param_info} {
    this->load_configuration();
  }

  LambdaTransform(O (*function)(I input, T1 param1), T1 param1,
                  const ParamInfo* param_info, String config_path = "")
      : Transform<I, O>(config_path),
        function1{function},
        param1{param1},
        num_params{1},
        param_info{param_info} {
    this->load_configuration();
  }

  /**
   * @brief LambdaTransform constructor for two-parameter function
   *
   * Construct a new LambdaTransform object which takes an input and two
   * configuration parameters. Multiple overloaded constructors
   * exist; their arguments can be extrapolated from this one.
   *
   * @param function Function to be executed for every new input value
   * @param param1 Configuration parameter 1
   * @param param2 Configuration parameter 2
   * @param param_info Keys and Descriptions of each configuration parameter
   * @param config_path Configuration path for the transform
   **/
  LambdaTransform(O (*function)(I input, T1 param1, T2 param2), T1 param1,
                  T2 param2, const ParamInfo* param_info,
                  String config_path = "")
      : Transform<I, O>(config_path),
        function2{function},
        param1{param1},
        param2{param2},
        num_params{2},
        param_info{param_info} {
    this->load_configuration();
  }

  LambdaTransform(O (*function)(I input, T1 param1, T2 param2, T3 param3),
                  T1 param1, T2 param2, T3 param3, const ParamInfo* param_info,
                  String config_path = "")
      : Transform<I, O>(config_path),
        function3{function},
        param1{param1},
        param2{param2},
        param3{param3},
        num_params{3},
        param_info{param_info} {
    this->load_configuration();
  }

  LambdaTransform(O (*function)(I input, T1 param1, T2 param2, T3 param3,
                                T4 param4),
                  T1 param1, T2 param2, T3 param3, T4 param4,
                  const ParamInfo* param_info, String config_path = "")
      : Transform<I, O>(config_path),
        function4{function},
        param1{param1},
        param2{param2},
        param3{param3},
        param4{param4},
        num_params{4},
        param_info{param_info} {
    this->load_configuration();
  }

  LambdaTransform(O (*function)(I input, T1 param1, T2 param2, T3 param3,
                                T4 param4, T5 param5),
                  T1 param1, T2 param2, T3 param3, T4 param4, T5 param5,
                  const ParamInfo* param_info, String config_path = "")
      : Transform<I, O>(config_path),
        function4{function},
        param1{param1},
        param2{param2},
        param3{param3},
        param4{param4},
        param4{param5},
        num_params{5},
        param_info{param_info} {
    this->load_configuration();
  }

  LambdaTransform(O (*function)(I input, T1 param1, T2 param2, T3 param3,
                                T4 param4, T5 param5, T6 param6),
                  T1 param1, T2 param2, T3 param3, T4 param4, T5 param5,
                  T6 param6, const ParamInfo* param_info,
                  String config_path = "")
      : Transform<I, O>(config_path),
        function4{function},
        param1{param1},
        param2{param2},
        param3{param3},
        param4{param4},
        param5{param5},
        param6{param6},
        num_params{6},
        param_info{param_info} {
    this->load_configuration();
  }

  void set_input(I input, uint8_t input_channel = 0) override {
    switch (num_params) {
      case 0:
        this->output = (*function0)(input);
        break;
      case 1:
        this->output = (*function1)(input, param1);
        break;
      case 2:
        this->output = (*function2)(input, param1, param2);
        break;
      case 3:
        this->output = (*function3)(input, param1, param2, param3);
        break;
      case 4:
        this->output = (*function4)(input, param1, param2, param3, param4);
        break;
      case 5:
        this->output =
            (*function5)(input, param1, param2, param3, param4, param5);
        break;
      case 6:
        this->output =
            (*function6)(input, param1, param2, param3, param4, param5, param6);
        break;
      default:
        break;
    }
    Observable::notify();
  }

  void get_configuration(JsonObject& doc) override {
    switch (num_params) {
      case 6:
        doc[param_info[5].key] = param6;
      case 5:
        doc[param_info[4].key] = param5;
      case 4:
        doc[param_info[3].key] = param4;
      case 3:
        doc[param_info[2].key] = param3;
      case 2:
        doc[param_info[1].key] = param2;
      case 1:
        doc[param_info[0].key] = param1;
      default:
        break;
    }
    doc["value"] = this->output;
  }

  bool set_configuration(const JsonObject& config) override {
    // test that each argument key (as defined by param_info)
    // exists in the received Json object
    debugD("Preparing to restore configuration from FS.");
    for (int i = 0; i < num_params; i++) {
      const char* expected = param_info[i].key;
      if (!config.containsKey(expected)) {
        debugD("Didn't find all keys.");
        return false;
      }
    }
    switch (num_params) {
      case 6:
        param6 = config[param_info[5].key];
      case 5:
        param5 = config[param_info[4].key];
      case 4:
        param4 = config[param_info[3].key];
      case 3:
        param3 = config[param_info[2].key];
      case 2:
        param2 = config[param_info[1].key];
      case 1:
        param1 = config[param_info[0].key];
      default:
        break;
    }
    debugD("Restored configuration");
    return true;
  }

  String get_config_schema() override {
    // FIXME: The heavy use of Strings puts a lot of faith in that class's
    // ability to not leak or corrupt memory. Would be better to receive
    // a character array pointer as an argument for writing the output to.
    String output = "";

    debugD("Preparing config schema");

    output.concat(FPSTR(kLambdaTransformSchemaHead));
    if (num_params > 0) {
      debugD("getting param_info:");
      debugD("%s -> %s", param_info[0].key, param_info[0].description);
      output.concat(format_schema_row(param_info[0].key,
                                      param_info[0].description,
                                      get_schema_type_string(param1), false));
      output.concat(",");
    }
    if (num_params > 1) {
      output.concat(format_schema_row(param_info[1].key,
                                      param_info[1].description,
                                      get_schema_type_string(param2), false));
      output.concat(",");
    }
    if (num_params > 2) {
      output.concat(format_schema_row(param_info[2].key,
                                      param_info[2].description,
                                      get_schema_type_string(param3), false));
      output.concat(",");
    }
    if (num_params > 3) {
      output.concat(format_schema_row(param_info[3].key,
                                      param_info[3].description,
                                      get_schema_type_string(param4), false));
      output.concat(",");
    }
    if (num_params > 4) {
      output.concat(format_schema_row(param_info[4].key,
                                      param_info[4].description,
                                      get_schema_type_string(param5), false));
      output.concat(",");
    }
    if (num_params > 5) {
      output.concat(format_schema_row(param_info[5].key,
                                      param_info[5].description,
                                      get_schema_type_string(param6), false));
      output.concat(",");
    }
    output.concat(format_schema_row(
        "value", "Last value", get_schema_type_string(this->output), true));
    output.concat(FPSTR(kLambdaTransformSchemaTail));

    debugD("Prepared config schema.");

    return output;
  }

 private:
  T1 param1;
  T2 param2;
  T3 param3;
  T4 param4;
  T5 param5;
  T6 param6;
  int num_params;
  const ParamInfo* param_info;

  O (*function0)(I input);
  O (*function1)(I input, T1 param1);
  O (*function2)(I input, T1 param1, T2 param2);
  O (*function3)(I input, T1 param1, T2 param2, T3 param3);
  O (*function4)(I input, T1 param1, T2 param2, T3 param3, T4 param4);
  O(*function5)
  (I input, T1 param1, T2 param2, T3 param3, T4 param4, T5 param5);
  O(*function6)
  (I input, T1 param1, T2 param2, T3 param3, T4 param4, T5 param5, T6 param6);

  static String format_schema_row(const char key[], const char title[],
                                  const char type[], const bool read_only) {
    char row[100] = "";
    const char* read_only_str = read_only ? "true" : "false";

    static const char schema_row[] = R"(
      "%s": { "title": "%s", "type": "%s", "readOnly": %s }
  )";

    sprintf(row, schema_row, key, title, type, read_only_str);

    return String(row);
  }
};

#endif
