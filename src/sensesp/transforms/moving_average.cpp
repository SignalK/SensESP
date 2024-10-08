#include "moving_average.h"

namespace sensesp {

// MovingAverage

MovingAverage::MovingAverage(int sample_size, float multiplier,
                             const String& config_path)
    : FloatTransform(config_path),
      sample_size_{sample_size},
      multiplier_{multiplier},
      initialized_(false) {

  buf_.resize(sample_size_, 0);

  load();
}

void MovingAverage::set(const float& input) {
  // So the first value to be included in the average doesn't default to 0.0
  if (!initialized_) {
    buf_.assign(sample_size_, input);
    output_ = input;
    initialized_ = true;
  } else {
    // Subtract 1/nth of the oldest value and add 1/nth of the newest value
    output_ += -multiplier_ * buf_[ptr_] / sample_size_;
    output_ += multiplier_ * input / sample_size_;

    // Save the most recent input, then advance to the next storage location.
    // When storage location n is reached, start over again at 0.
    buf_[ptr_] = input;
    ptr_ = (ptr_ + 1) % sample_size_;
  }
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
  multiplier_ = config["multiplier"];
  int const n_new = config["sample_size"];
  // need to reset the ring buffer if size changes
  if (sample_size_ != n_new) {
    buf_.assign(sample_size_, 0);
    ptr_ = 0;
    initialized_ = false;
    sample_size_ = n_new;
  }
  return true;
}
const String ConfigSchema(const MovingAverage& obj) {
  return R"({"type":"object","properties":{"sample_size":{"title":"Number of samples in average","type":"integer"},"multiplier":{"title":"Multiplier","type":"number"}}})";
}

}  // namespace sensesp
