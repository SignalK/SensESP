#include "moving_average.h"

// MovingAverage

MovingAverage::MovingAverage(int sample_size, float multiplier, String config_path)
    : NumericTransform(config_path), sample_size_{sample_size}, multiplier_{multiplier} {
  buf_.resize(sample_size_, 0);
  initialized_ = false;
  load_configuration();
}

void MovingAverage::set_input(float input, uint8_t inputChannel) {
  // So the first value to be included in the average doesn't default to 0.0
  if (!initialized_) {
    buf_.assign(sample_size_, input);
    output = input;
    initialized_ = true;
  } else {
    // Subtract 1/nth of the oldest value and add 1/nth of the newest value
    output += -multiplier_ * buf_[ptr_] / sample_size_;
    output += multiplier_ * input / sample_size_;

    // Save the most recent input, then advance to the next storage location.
    // When storage location n is reached, start over again at 0.
    buf_[ptr_] = input;
    ptr_ = (ptr_ + 1) % sample_size_;
  }
  notify();
}

void MovingAverage::get_configuration(JsonObject& root) {
  root["multiplier"] = multiplier_;
  root["sample_size"] = sample_size_;
}

static const char SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "sample_size": { "title": "Number of samples in average", "type": "integer" },
        "multiplier": { "title": "Multiplier", "type": "number" }
    }
  })";

String MovingAverage::get_config_schema() { return FPSTR(SCHEMA); }

bool MovingAverage::set_configuration(const JsonObject& config) {
  String expected[] = {"multiplier", "sample_size"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  multiplier_ = config["multiplier"];
  int n_new = config["sample_size"];
  // need to reset the ring buffer if size changes
  if (sample_size_ != n_new) {
    buf_.assign(sample_size_, 0);
    ptr_ = 0;
    initialized_ = false;
    sample_size_ = n_new;
  }
  return true;
}
