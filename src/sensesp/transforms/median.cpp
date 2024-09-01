#include "median.h"

namespace sensesp {

Median::Median(unsigned int sample_size, const String& config_path)
    : FloatTransform(config_path), sample_size_{sample_size} {
  set_requires_restart(true);
  load_configuration();
  buf_.reserve(sample_size_);
  buf_.clear();
}

void Median::set(const float& input) {
  buf_.push_back(input);
  if (buf_.size() >= sample_size_) {
    // Its time to output a value
    sort(buf_.begin(), buf_.end());
    const unsigned int mid = sample_size_ / 2;
    output = buf_[mid];
    buf_.clear();
    notify();
  }
}

void Median::get_configuration(JsonObject& root) {
  root["sample_size"] = sample_size_;
}

static const char kSchema[] = R"({
    "type": "object",
    "properties": {
        "sample_size": { "title": "Sample size", "description": "Number of samples to take before outputing a value", "type": "integer" }
    }
  })";

String Median::get_config_schema() { return (kSchema); }

bool Median::set_configuration(const JsonObject& config) {
  String const expected[] = {"sample_size"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  unsigned int const sample_size_new = config["sample_size"];
  if (sample_size_ != sample_size_new) {
    sample_size_ = sample_size_new;
    buf_.reserve(sample_size_);
    buf_.clear();
  }
  return true;
}

}  // namespace sensesp
