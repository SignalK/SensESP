#include "median.h"

namespace sensesp {

Median::Median(unsigned int sample_size, const String& config_path)
    : FloatTransform(config_path), sample_size_{sample_size} {
  load();
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

bool Median::to_json(JsonObject& root) {
  root["sample_size"] = sample_size_;
  return true;
}

bool Median::from_json(const JsonObject& config) {
  String const expected[] = {"sample_size"};
  for (auto str : expected) {
    if (!config[str].is<JsonVariant>()) {
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

const String ConfigSchema(const Median& obj) {
  return R"({"type":"object","properties":{"sample_size":{"title":"Sample size","description":"Number of samples to take before outputing a value","type":"integer"}}})";
}

}  // namespace sensesp
