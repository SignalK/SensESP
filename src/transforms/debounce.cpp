#include "debounce.h"

template<class T>
Debounce<T>::Debounce(int ms_min_delay, String config_path)
    : SymmetricTransform<T>(config_path), ms_min_delay{ms_min_delay} {
      interrupt_timer = 0;
}

template<class T>
void Debounce<T>::set_input(T newValue, uint8_t inputChannel) {
  if (interrupt_timer > ms_min_delay) {
    interrupt_timer = 0;
    this->emit(newValue);
  }
}

template<class T>
void Debounce<T>::get_configuration(JsonObject& root) {
  root["min_delay"] = ms_min_delay;
}

static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "min_delay": { "title": "Minimum delay", "type": "number", "description": "The minimum time in ms between inputs for output to happen" }
    }
  })###";

template<class T>
String Debounce<T>::get_config_schema() { return FPSTR(SCHEMA); }

template<class T>
bool Debounce<T>::set_configuration(const JsonObject& config) {
  String expected[] = {"min_delay"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  ms_min_delay = config["min_delay"];
  return true;
}

// define all possible instances of a Debounce
template class Debounce<bool>;
template class Debounce<int>;
template class Debounce<float>;
template class Debounce<String>;