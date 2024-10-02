#ifndef SENSESP_SRC_TRANSFORMS_TIME_COUNTER_H_
#define SENSESP_SRC_TRANSFORMS_TIME_COUNTER_H_

#include "sensesp/ui/config_item.h"
#include "sensesp/transforms/transform.h"

namespace sensesp {

inline uint64_t ARDUINO_ISR_ATTR millis64() { return esp_timer_get_time() / 1000ULL; }

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
class TimeCounter : public Transform<T, double> {
 public:
  TimeCounter(String config_path) : Transform<T, double>(config_path) {
    this->load();
  }

  virtual void set(const T& input) override {
    if (!initialized_) {
      // Initialize the previous state
      initialized_ = true;
      previous_state_ = (bool)input;
      start_time_ms_ = millis64();
      duration_at_start_ms_ = duration_ms_;
    }

    // if previous_state_ is true, accumulate duration
    if (previous_state_) {
      duration_ms_ = duration_at_start_ms_ + (millis64() - start_time_ms_);
    }

    if (input) {
      if (previous_state_ == 0) {
        // State change from false to true
        previous_state_ = 1;
        start_time_ms_ = millis64();
        duration_at_start_ms_ = duration_ms_;
        this->save();  // Save configuration to flash, so that
                       // the duration is persistent
      }
    } else {
      if (previous_state_ == 1) {
        // State change from true to false
        previous_state_ = 0;
        duration_ms_ = duration_at_start_ms_ + (millis64() - start_time_ms_);
        this->save();  // Save configuration to flash, so that
                       // the duration is persistent
      }
    }
    this->emit((double)duration_ms_ / 1000.);
  }

  virtual bool to_json(JsonObject& root) override {
    root["duration_s"] = duration_ms_ / 1000.;  // convert to seconds
    return true;
  }

  virtual bool from_json(const JsonObject& config) override {
    ESP_LOGD(__FILENAME__, "Setting TimeCounter configuration");
    if (config["duration_s"].is<double>()) {
      duration_at_start_ms_ =
          config["duration_s"].as<double>() * 1000;  // convert to milliseconds
    } else if (!config["duration"].is<double>()) {
      // If the duration is not in seconds, try to read it in milliseconds
      duration_at_start_ms_ = config["duration"];
    } else {
      return false;
    }

    duration_ms_ = duration_at_start_ms_;
    ESP_LOGD(__FILENAME__, "duration_at_start_ms_ = %ld",
             duration_at_start_ms_);
    return true;
  }

 protected:
  bool initialized_ = false;
  bool previous_state_ = false;
  uint64_t start_time_ms_;
  uint64_t duration_ms_ = 0;
  uint64_t duration_at_start_ms_ = 0;
};

template <typename U>
const String ConfigSchema(TimeCounter<U>& obj) {
  return R"({
      "type": "object",
      "properties": {
          "duration_s": {
            "type": "number",
            "displayMultiplier": 0.0002777777777777778,
            "title": "Total Duration [hours]"
          }
      },
      "required": ["duration_s"]
    })";
}

}  // namespace sensesp

#endif  // SENSESP_TRANSFORMS_TIME_COUNTER_H_
