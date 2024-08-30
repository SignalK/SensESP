#ifndef SENSESP_TRANSFORMS_LAMBDA_TRANSFORM_H_
#define SENSESP_TRANSFORMS_LAMBDA_TRANSFORM_H_

#include "transform.h"

namespace sensesp {

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
// an unknown type may not be rendered in Json editor
template <class T>
const char* get_schema_type_string(const T dummy) {
  return "unknown";
}
template <>
const char* get_schema_type_string(const int dummy);

template <>
const char* get_schema_type_string(const float dummy);

template <>
const char* get_schema_type_string(const String dummy);

template <>
const char* get_schema_type_string(const bool dummy);

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
 * @tparam IN Transform function input value type
 * @tparam OUT Transform function output value type
 * @tparam P1 Transform function parameter 1 type
 * @tparam P2 Transform function parameter 2 type
 * @tparam P3 Transform function parameter 3 type
 * @tparam P4 Transform function parameter 4 type
 * @tparam P5 Transform function parameter 5 type
 * @tparam P6 Transform function parameter 6 type
 * */
template <class IN, class OUT, class P1 = bool, class P2 = bool,
          class P3 = bool, class P4 = bool, class P5 = bool, class P6 = bool>
class LambdaTransform : public Transform<IN, OUT> {
 public:
  LambdaTransform(std::function<OUT(IN)> function, const String& config_path = "")
      : Transform<IN, OUT>(config_path), num_params_{0}, function0_{function} {
    this->load_configuration();
  }

  LambdaTransform(std::function<OUT(IN)> function, const ParamInfo* param_info,
                  const String& config_path = "")
      : Transform<IN, OUT>(config_path),
        num_params_{0},
        function0_{function},
        param_info_{param_info} {
    this->load_configuration();
  }

  LambdaTransform(std::function<OUT(IN, P1)> function, P1 param1,
                  const ParamInfo* param_info, const String& config_path = "")
      : Transform<IN, OUT>(config_path),
        num_params_{1},
        function1_{function},
        param1_{param1},
        param_info_{param_info} {
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
  LambdaTransform(std::function<OUT(IN, P1, P2)> function, P1 param1, P2 param2,
                  const ParamInfo* param_info, const String& config_path = "")
      : Transform<IN, OUT>(config_path),
        num_params_{2},
        function2_{function},
        param1_{param1},
        param2_{param2},
        param_info_{param_info} {
    this->load_configuration();
  }

  LambdaTransform(std::function<OUT(IN, P1, P2, P3)> function, P1 param1,
                  P2 param2, P3 param3, const ParamInfo* param_info,
                  const String& config_path = "")
      : Transform<IN, OUT>(config_path),
        num_params_{3},
        function3_{function},
        param1_{param1},
        param2_{param2},
        param3_{param3},
        param_info_{param_info} {
    this->load_configuration();
  }

  LambdaTransform(std::function<OUT(IN, P1, P2, P3, P4)> function, P1 param1,
                  P2 param2, P3 param3, P4 param4, const ParamInfo* param_info,
                  const String& config_path = "")
      : Transform<IN, OUT>(config_path),
        num_params_{4},
        function4_{function},
        param1_{param1},
        param2_{param2},
        param3_{param3},
        param4_{param4},
        param_info_{param_info} {
    this->load_configuration();
  }

  LambdaTransform(std::function<OUT(IN, P1, P2, P3, P4, P5)> function,
                  P1 param1, P2 param2, P3 param3, P4 param4, P5 param5,
                  const ParamInfo* param_info, const String& config_path = "")
      : Transform<IN, OUT>(config_path),
        num_params_{5},
        function5_{function},
        param1_{param1},
        param2_{param2},
        param3_{param3},
        param4_{param4},
        param5_{param5},
        param_info_{param_info} {
    this->load_configuration();
  }

  LambdaTransform(std::function<OUT(IN, P1, P2, P3, P4, P5, P6)> function,
                  P1 param1, P2 param2, P3 param3, P4 param4, P5 param5,
                  P6 param6, const ParamInfo* param_info,
                  const String& config_path = "")
      : Transform<IN, OUT>(config_path),
        num_params_{6},
        function6_{function},
        param1_{param1},
        param2_{param2},
        param3_{param3},
        param4_{param4},
        param5_{param5},
        param6_{param6},
        param_info_{param_info} {
    this->load_configuration();
  }

  void set(const IN& input) override {
    switch (num_params_) {
      case 0:
        this->output = function0_(input);
        break;
      case 1:
        this->output = function1_(input, param1_);
        break;
      case 2:
        this->output = function2_(input, param1_, param2_);
        break;
      case 3:
        this->output = function3_(input, param1_, param2_, param3_);
        break;
      case 4:
        this->output = function4_(input, param1_, param2_, param3_, param4_);
        break;
      case 5:
        this->output = function5_(input, param1_, param2_, param3_, param4_, param5_);
        break;
      case 6:
        this->output =
            function6_(input, param1_, param2_, param3_, param4_, param5_, param6_);
        break;
      default:
        break;
    }
    Observable::notify();
  }

  void get_configuration(JsonObject& doc) override {
    switch (num_params_) {
      case 6:
        doc[param_info_[5].key] = param6_;
      case 5:
        doc[param_info_[4].key] = param5_;
      case 4:
        doc[param_info_[3].key] = param4_;
      case 3:
        doc[param_info_[2].key] = param3_;
      case 2:
        doc[param_info_[1].key] = param2_;
      case 1:
        doc[param_info_[0].key] = param1_;
      default:
        break;
    }
  }

  bool set_configuration(const JsonObject& config) override {
    // test that each argument key (as defined by param_info)
    // exists in the received Json object
    ESP_LOGD(__FILENAME__, "Preparing to restore configuration from FS.");
    for (int i = 0; i < num_params_; i++) {
      const char* expected = param_info_[i].key;
      if (!config.containsKey(expected)) {
        ESP_LOGD(__FILENAME__, "Didn't find all keys.");
        return false;
      }
    }
    switch (num_params_) {
      case 6:
        param6_ = config[param_info_[5].key];
      case 5:
        param5_ = config[param_info_[4].key];
      case 4:
        param4_ = config[param_info_[3].key];
      case 3:
        param3_ = config[param_info_[2].key];
      case 2:
        param2_ = config[param_info_[1].key];
      case 1:
        param1_ = config[param_info_[0].key];
      default:
        break;
    }
    ESP_LOGD(__FILENAME__, "Restored configuration");
    return true;
  }

  String get_config_schema() override {
    // FIXME: The heavy use of Strings puts a lot of faith in that class's
    // ability to not leak or corrupt memory. Would be better to receive
    // a character array pointer as an argument for writing the output to.
    String output = "";

    if (num_params_ == 0) {
      return "{}";
    }

    ESP_LOGD(__FILENAME__, "Preparing config schema for %d parameters",
             num_params_);

    output.concat(FPSTR(kLambdaTransformSchemaHead));
    if (num_params_ > 0) {
      ESP_LOGD(__FILENAME__, "getting param_info:");
      ESP_LOGD(__FILENAME__, "%s -> %s", param_info_[0].key,
               param_info_[0].description);
      output.concat(format_schema_row(param_info_[0].key,
                                      param_info_[0].description,
                                      get_schema_type_string(param1_), false));
    }
    if (num_params_ > 1) {
      output.concat(",");
      output.concat(format_schema_row(param_info_[1].key,
                                      param_info_[1].description,
                                      get_schema_type_string(param2_), false));
    }
    if (num_params_ > 2) {
      output.concat(",");
      output.concat(format_schema_row(param_info_[2].key,
                                      param_info_[2].description,
                                      get_schema_type_string(param3_), false));
    }
    if (num_params_ > 3) {
      output.concat(",");
      output.concat(format_schema_row(param_info_[3].key,
                                      param_info_[3].description,
                                      get_schema_type_string(param4_), false));
    }
    if (num_params_ > 4) {
      output.concat(",");
      output.concat(format_schema_row(param_info_[4].key,
                                      param_info_[4].description,
                                      get_schema_type_string(param5_), false));
    }
    if (num_params_ > 5) {
      output.concat(",");
      output.concat(format_schema_row(param_info_[5].key,
                                      param_info_[5].description,
                                      get_schema_type_string(param6_), false));
    }
    output.concat(FPSTR(kLambdaTransformSchemaTail));

    ESP_LOGD(__FILENAME__, "Prepared config schema.");

    return output;
  }

 private:
  int num_params_;

  std::function<OUT(IN)> function0_;
  std::function<OUT(IN, P1)> function1_;
  std::function<OUT(IN, P1, P2)> function2_;
  std::function<OUT(IN, P1, P2, P3)> function3_;
  std::function<OUT(IN, P1, P2, P3, P4)> function4_;
  std::function<OUT(IN, P1, P2, P3, P4, P5)> function5_;
  std::function<OUT(IN, P1, P2, P3, P4, P5, P6)> function6_;

  P1 param1_;
  P2 param2_;
  P3 param3_;
  P4 param4_;
  P5 param5_;
  P6 param6_;

  const ParamInfo* param_info_;

  static String format_schema_row(const char key[], const char title[],
                                  const char type[], const bool read_only) {
    char row[1024] = "";
    const char* read_only_str = read_only ? "true" : "false";

    static const char schema_row[] = R"(
      "%s": { "title": "%s", "type": "%s", "readOnly": %s }
  )";

    sprintf(row, schema_row, key, title, type, read_only_str);

    ESP_LOGD(__FILENAME__, "Formatted schema row: %s", row);

    return String(row);
  }
};

}  // namespace sensesp

#endif
