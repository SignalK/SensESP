#include "change_filter.h"

static float absf(float val) {
  if (val < 0) {
    return -val;
  } else {
    return val;
  }
}

ChangeFilter::ChangeFilter(float min_delta, float max_delta, int max_skips,
                           String config_path)
    : NumericTransform(config_path),
      min_delta{min_delta},
      max_delta{max_delta},
      max_skips{max_skips} {
  load_configuration();
  skips = max_skips + 1;
}

void ChangeFilter::set_input(float new_value, uint8_t input_channel) {
  float delta = absf(new_value - output);
  if ((delta >= min_delta && delta <= max_delta) || skips > max_skips) {
    output = new_value;
    skips = 0;
    notify();
  } else {
    skips++;
  }
}

void ChangeFilter::get_configuration(JsonObject& root) {
  root["min_delta"] = min_delta;
  root["max_delta"] = max_delta;
  root["max_skips"] = max_skips;
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
  min_delta = config["min_delta"];
  max_delta = config["max_delta"];
  max_skips = config["max_skips"];
  skips = max_skips + 1;
  return true;
}
