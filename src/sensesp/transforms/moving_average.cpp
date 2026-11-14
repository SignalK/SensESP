#include "moving_average.h"

namespace sensesp {

// MovingAverage

MovingAverage::MovingAverage(int sample_size, float multiplier,
                             const String& config_path)
    : FloatTransform(config_path),
      sample_size_{sample_size > 0 ? sample_size : 1},
      multiplier_{multiplier},
      initialized_(false) {

  buf_.resize(sample_size_, 0);

  load();
}

void MovingAverage::set(const float& input) {
  if (!initialized_) {
    // Seed the whole buffer with the first value so the average doesn't start
    // out diluted by zeros.
    buf_.assign(sample_size_, input);
    sum_ = static_cast<float>(sample_size_) * input;
    initialized_ = true;
  } else {
    // Swap the oldest buffered value out of the running sum for the newest.
    sum_ += input - buf_[ptr_];
    buf_[ptr_] = input;
    ptr_ = (ptr_ + 1) % sample_size_;
  }
  output_ = multiplier_ * sum_ / sample_size_;
  notify();
}

bool MovingAverage::to_json(JsonObject& root) {
  root["multiplier"] = multiplier_;
  root["sample_size"] = sample_size_;
  return true;
}

bool MovingAverage::from_json(const JsonObject& config) {
  String const expected[] = {"multiplier", "sample_size"};
  for (auto str : expected) {
    if (!config[str].is<JsonVariant>()) {
      return false;
    }
  }
  if (!config["sample_size"].is<int>() ||
      config["sample_size"].as<int>() <= 0) {
    return false;
  }
  multiplier_ = config["multiplier"];
  int const n_new = config["sample_size"];
  // need to reset the ring buffer if size changes
  if (sample_size_ != n_new) {
    sample_size_ = n_new;
    buf_.assign(sample_size_, 0);
    ptr_ = 0;
    initialized_ = false;
  }
  return true;
}
const String ConfigSchema(const MovingAverage& obj) {
  return R"({"type":"object","properties":{"sample_size":{"title":"Number of samples in average","type":"integer","minimum":1},"multiplier":{"title":"Multiplier","type":"number"}}})";
}

}  // namespace sensesp
