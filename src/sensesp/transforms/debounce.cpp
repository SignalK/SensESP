#include "debounce.h"

namespace sensesp {

const String ConfigSchema(const Debounce<bool>& obj) {
  return R"###({
    "type": "object",
    "properties": {
        "min_delay": { "title": "Minimum delay", "type": "number", "description": "The minimum time in ms between inputs for output to happen" }
    }
  })###";
}

}  // namespace sensesp
