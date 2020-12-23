#include "transforms/repeat_report.h"

template <typename T>
void RepeatReport<T>::set_input(T input, uint8_t inputChannel) {
  last_update_interval_ = 0;
  this->emit(input);
}

template <typename T>
void RepeatReport<T>::enable() {
  SymmetricTransform<T>::enable();
  app.onRepeat(10, [this]() {
    if (last_update_interval_ > max_silence_interval_) {
      this->last_update_interval_ = 0;
      this->notify();
    }
  });
}

template <typename T>
void RepeatReport<T>::get_configuration(JsonObject& root) {
  root["max_silence_interval"] = max_silence_interval_;
}

static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "max_silence_interval": { "title": "Max ms interval until value repeated", "type": "number" }
    }
  })###";

template <typename T>
String RepeatReport<T>::get_config_schema() {
  return FPSTR(SCHEMA);
}

template <typename T>
bool RepeatReport<T>::set_configuration(const JsonObject& config) {
  String expected[] = {"max_silence_interval"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  this->max_silence_interval_ = config["max_silence_interval"];
  return true;
}

// Force compiler to make versions for the common data types...
template class RepeatReport<float>;
template class RepeatReport<int>;
template class RepeatReport<bool>;
template class RepeatReport<String>;
