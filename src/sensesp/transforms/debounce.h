#ifndef SENSESP_TRANSFORMS_DEBOUNCE_H_
#define SENSESP_TRANSFORMS_DEBOUNCE_H_

#include "sensesp/ui/config_item.h"
#include "transform.h"

namespace sensesp {

/**
 * @brief Implements debounce code for a button or switch
 *
 * It's a passthrough transform that will output a value only if it's different
 * from the previous output, and only if it's been ms_min_delay ms since the
 * input was received.
 *
 * @tparam T The type of value being passed through Debounce.
 *
 * @param ms_min_delay The minimum amount of time that must have passed since
 * the input was received by this Transform in order for the output to occur. If
 * you are using this to debounce the output from `DigitalInputChange`,
 * ms_min_delay should be set at least a little bit longer than
 * `DigitalInputChange::read_delay`.
 *
 * DigitalInputChange::read_delay is 10 ms by default, and
 * Debounce::ms_min_delay is 15 ms by default. If that doesn't adequately
 * debounce your button or switch, adjust both of those values until it does.
 *
 * @param config_path The path for configuring ms_min_delay with the Config UI.
 * @see DigitalInputChange
 */
template <class T>
class Debounce : public SymmetricTransform<T> {
 public:
  Debounce(int ms_min_delay = 15, String config_path = "")
      : SymmetricTransform<T>(config_path), ms_min_delay_{ms_min_delay} {
    this->load();
  }

  virtual void set(const T& input) override {
    // Input has changed since the last emit, or this is the first
    // input since the program started to run.

    if (input != debounced_value_ || !value_received_) {
      debounced_value_ = input;

      if (event_) {
        event_->remove(event_loop());
        event_ = nullptr;
      }
      event_ = event_loop()->onDelay(ms_min_delay_, [this, input]() {
        this->event_ = nullptr;
        this->debounced_value_ = input;
        this->emit(input);
      });
      value_received_ = true;
    }
  }

  virtual bool to_json(JsonObject& doc) override {
    doc["min_delay"] = ms_min_delay_;
    return true;
  }

  virtual bool from_json(const JsonObject& config) override {
    const String expected[] = {"min_delay"};
    for (auto str : expected) {
      if (!config[str].is<JsonVariant>()) {
        return false;
      }
    }
    ms_min_delay_ = config["min_delay"];
    return true;
  }

 protected:
  int ms_min_delay_;
  bool value_received_ = false;
  T debounced_value_;
  reactesp::DelayEvent* event_ = nullptr;
};

template <typename T>
const String ConfigSchema(const Debounce<T>& obj) {
  return R"###({"type":"object","properties":{"min_delay":{"title":"Minimum delay","type":"number","description":"The minimum time in ms between inputs for output to happen"}}})###";
}

typedef Debounce<bool> DebounceBool;
typedef Debounce<int> DebounceInt;
typedef Debounce<float> DebounceFloat;

}  // namespace sensesp

#endif
