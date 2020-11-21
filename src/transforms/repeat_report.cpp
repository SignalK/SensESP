#include "transforms/repeat_report.h"

template <typename T>
void RepeatReport<T>::set_input(T input, uint8_t inputChannel) {
      last_update = millis();
      this->emit(input);
}


template <typename T>
void RepeatReport<T>::enable() {
      SymmetricTransform<T>::enable();
      app.onTick([this]() {
         unsigned long interval = millis() - this->last_update;
         if (interval > max_silence_interval) {
           this->last_update = millis();
           this->notify();
         }
      });
}


template <typename T>
void RepeatReport<T>::get_configuration(JsonObject& root) {
  root["max_silence_interval"] = max_silence_interval;
  root["value"] = this->output;
}

static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "max_silence_interval": { "title": "Max ms interval until value repeated", "type": "number" },
        "value": { "title": "Last value", "type" : "number", "readOnly": true }
    }
  })###";

template <typename T>
String RepeatReport<T>::get_config_schema() { return FPSTR(SCHEMA); }

template <typename T>
bool RepeatReport<T>::set_configuration(const JsonObject& config) {
  String expected[] = {"max_silence_interval"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  this->max_silence_interval = config["max_silence_interval"];
  return true;
}


// Force compiler to make versions for the common data types...
template class RepeatReport<float>;
template class RepeatReport<int>;
template class RepeatReport<bool>;
template class RepeatReport<String>;
