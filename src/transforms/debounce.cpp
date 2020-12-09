#include "debounce.h"

// Developers: this isn't an ideal implementation of a templated Transform.
// See the limitations and suggestion solution in SensESP Issue #287.
// https://github.com/SignalK/SensESP/issues/287

template <class T>
DebounceTemplate<T>::DebounceTemplate(int ms_min_delay, String config_path)
    : SymmetricTransform<T>(config_path),
      ms_min_delay_{ms_min_delay},
      value_sent_{false},
      stable_input_{false},
      reaction_{NULL} {
  this->load_configuration();
}

template <class T>
void DebounceTemplate<T>::set_input(T input, uint8_t inputChannel) {
  if (reaction_ != NULL) {
    reaction_->remove();
    reaction_ = NULL;
  }
  // Input has changed since the last emit, or this is the first
  // input since the program started to run.
  if (input != stable_input_ || value_sent_ == false) {
    reaction_ = app.onDelay(ms_min_delay_, [this, input]() {
      this->reaction_ = NULL;
      this->stable_input_ = input;
      this->value_sent_ = true;
      this->emit(input);
    });
  } 
}

template<class T>
void DebounceTemplate<T>::get_configuration(JsonObject& root) {
  root["min_delay"] = ms_min_delay_;
}

static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "min_delay": { "title": "Minimum delay", "type": "number", "description": "The minimum time in ms between inputs for output to happen" }
    }
  })###";

template<class T>
String DebounceTemplate<T>::get_config_schema() { return FPSTR(SCHEMA); }

template<class T>
bool DebounceTemplate<T>::set_configuration(const JsonObject& config) {
  String expected[] = {"min_delay"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  ms_min_delay_ = config["min_delay"];
  return true;
}

// define all possible instances of a Debounce
template class DebounceTemplate<bool>;
template class DebounceTemplate<int>;
template class DebounceTemplate<float>;
//template class DebounceTemplate<String>;
