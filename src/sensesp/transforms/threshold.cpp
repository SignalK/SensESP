#include "threshold.h"

namespace sensesp {

static const char kSchemaTemplate[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "min": { "title": "Minimum value", "type": "!!TYPE!!", "description" : "Minimum value to be 'in range'" },
        "max": { "title": "Maximum value", "type": "!!TYPE!!", "description" : "Maximum value to be 'in range'" },
        "in_range": { "title": "In range value", "type": "boolean", "description" : "Output value when input value is 'in range'" }
        "out_range": { "title": "Out of range value", "type": "boolean", "description" : "Output value when input value is out of range" }
    }
  })";

const String ConfigSchema(const ThresholdTransform<float>& obj) {
  String schema = kSchemaTemplate;
  schema.replace("!!TYPE!!", "number");
  return schema;
}

const String ConfigSchema(const ThresholdTransform<int>& obj) {
  String schema = kSchemaTemplate;
  schema.replace("!!TYPE!!", "integer");
  return schema;
}

}  // namespace sensesp
