#include "debounce.h"

template<class T>
DebounceTemplate<T>::DebounceTemplate(int ms_min_delay, String config_path)
    : SymmetricTransform<T>(config_path), ms_min_delay_{ms_min_delay} {
      interrupt_timer_ = 0;
}

template<class T>
void DebounceTemplate<T>::set_input(T newValue, uint8_t inputChannel) {
  if (interrupt_timer_ > ms_min_delay_) {
    interrupt_timer_ = 0;
    this->emit(newValue);
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
template class DebounceTemplate<String>;