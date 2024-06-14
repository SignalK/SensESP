#include "change_filter.h"

namespace sensesp {

static float absf(float val) {
  if (val < 0) {
    return -val;
  } else {
    return val;
  }
}

ChangeFilter::ChangeFilter(float min_delta, float max_delta, int max_skips,
                           String config_path)
    : FloatTransform(config_path),
      min_delta_{min_delta},
      max_delta_{max_delta},
      max_skips_{max_skips} {
  load_configuration();
  skips_ = max_skips_ + 1;
}

void ChangeFilter::set(float new_value) {
  float delta = absf(new_value - output);
  if ((delta >= min_delta_ && delta <= max_delta_) || skips_ > max_skips_) {
    skips_ = 0;
    this->emit(new_value);
  } else {
    skips_++;
  }
}

void ChangeFilter::get_configuration(JsonObject& root) {
  root["min_delta"] = min_delta_;
  root["max_delta"] = max_delta_;
  root["max_skips"] = max_skips_;
}

static const char SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "min_delta": { "title": "Minimum delta", "description": "Minimum difference in change of value before forwarding", "type": "number" },
        "max_delta": { "title": "Maximum delta", "description": "Maximum difference in change of value to allow forwarding", "type": "number" },
        "max_skips": { "title": "Max skip count", "description": "Maximum number of consecutive filtered values before one is allowed through", "type": "number" }
    }
  })";

String ChangeFilter::get_config_schema() { return FPSTR(SCHEMA); }

bool ChangeFilter::set_configuration(const JsonObject& config) {
  String expected[] = {"min_delta", "max_delta", "max_skips"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  min_delta_ = config["min_delta"];
  max_delta_ = config["max_delta"];
  max_skips_ = config["max_skips"];
  skips_ = max_skips_ + 1;
  return true;
}

}  // namespace sensesp
