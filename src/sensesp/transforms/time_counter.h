#ifndef SENSESP_SRC_TRANSFORMS_TIME_COUNTER_H_
#define SENSESP_SRC_TRANSFORMS_TIME_COUNTER_H_

#include "sensesp/transforms/transform.h"

namespace sensesp {

static const char kTimeCounterSchema[] = R"({
    "type": "object",
    "properties": {
        "duration": {
          "type": "number",
          "title": "Total Duration",
          "description": "Total accumulated duration while the input state is non-zero or true, in seconds"
        }
    },
    "required": ["duration"]

})";

/**
 * @brief A transform that outputs the duration of the input value being
 * true or non-null.
 *
 * The main use case for this transform is to measure the total engine hours
 * in a persistent way. The value is stored in the flash drive whenever the
 * input state changes (the engine is turned on or off).
 *
 * @tparam T The type of the input value. Must be castable to a boolean.
 */
template <typename T>
class TimeCounter : public Transform<T, float> {
 public:
  TimeCounter(String config_path) : Transform<T, float>(config_path) {
    this->load_configuration();
  }

  virtual void set(T input, uint8_t input_channel = 0) override {
    if (previous_state_ == -1) {
      // Initialize the previous state
      previous_state_ = (bool)input;
      start_time_ = millis();
      duration_at_start_ = duration_;
    }

    // if previous_state_ is true, accumulate duration
    if (previous_state_) {
      duration_ = duration_at_start_ + (millis() - start_time_);
    }

    if (input) {
      if (previous_state_ == 0) {
        // State change from false to true
        previous_state_ = 1;
        start_time_ = millis();
        duration_at_start_ = duration_;
      }
    } else {
      if (previous_state_ == 1) {
        // State change from true to false
        previous_state_ = 0;
        duration_ = duration_at_start_ + (millis() - start_time_);
        this->save_configuration();  // Save configuration to flash, so that
                                     // the duration is persistent
      }
    }
    this->emit((float)duration_ / 1000.);
  }

  virtual void get_configuration(JsonObject& root) override {
    root["duration"] = duration_;
  }

  virtual bool set_configuration(const JsonObject& config) override {
    debugD("Setting TimeCounter configuration");
    if (!config.containsKey("duration")) {
      return false;
    }
    duration_at_start_ = config["duration"];
    duration_ = duration_at_start_;
    debugD("duration_at_start_ = %ld", duration_at_start_);
    return true;
  }

  virtual String get_config_schema() override { return kTimeCounterSchema; }

 protected:
  int previous_state_ = -1;  // -1 means uninitialized
  unsigned long start_time_;
  unsigned long duration_ = 0.;
  unsigned long duration_at_start_ = 0.;
};

}  // namespace sensesp

#endif  // SENSESP_TRANSFORMS_TIME_COUNTER_H_
