#include "change_filter.h"

#include <cmath>

namespace sensesp {

ChangeFilter::ChangeFilter(float min_delta, float max_delta, int max_skips,
                           const String& config_path)
    : FloatTransform(config_path),
      min_delta_{min_delta},
      max_delta_{max_delta},
      max_skips_{max_skips},
      skips_(max_skips_) {
  load();
}

void ChangeFilter::set(const float& new_value) {
  if (skips_ >= max_skips_ ||
      (fabsf(new_value - output_) >= min_delta_ &&
       fabsf(new_value - output_) <= max_delta_)) {
    skips_ = 0;
    this->emit(new_value);
  } else {
    skips_++;
  }
}

bool ChangeFilter::to_json(JsonObject& root) {
  root["min_delta"] = min_delta_;
  root["max_delta"] = max_delta_;
  root["max_skips"] = max_skips_;
  return true;
}

bool ChangeFilter::from_json(const JsonObject& config) {
  String const expected[] = {"min_delta", "max_delta", "max_skips"};
  for (auto str : expected) {
    if (!config[str].is<JsonVariant>()) {
      return false;
    }
  }
  min_delta_ = config["min_delta"];
  max_delta_ = config["max_delta"];
  max_skips_ = config["max_skips"];
  skips_ = max_skips_;
  return true;
}

const String ConfigSchema(ChangeFilter& obj) {
  return R"###({"type":"object","properties":{"min_delta":{"title":"Minimum delta","description":"Minimum difference in change of value before forwarding","type":"number"},"max_delta":{"title":"Maximum delta","description":"Maximum difference in change of value to allow forwarding","type":"number"},"max_skips":{"title":"Max skip count","description":"Maximum number of consecutive filtered values before one is allowed through","type":"number"}}})###";
}

}  // namespace sensesp
