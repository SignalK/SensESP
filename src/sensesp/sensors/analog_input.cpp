#include "sensesp.h"

#include "analog_input.h"

#include "Arduino.h"
#include "sensesp_base_app.h"

namespace sensesp {

AnalogInput::AnalogInput(uint8_t pin, unsigned int read_delay,
                         const String& config_path, float output_scale)
    : FloatSensor(config_path),
      pin_{pin},
      read_delay_{read_delay},
      output_scale_{output_scale} {
  analog_reader_ = std::unique_ptr<AnalogReader>(new AnalogReader(pin_));
  // load() may replace read_delay_ with the value persisted in the file
  // system, so arm the timer only after it has run.
  load();

  repeat_event_ =
      event_loop()->onRepeat(read_delay_, [this]() { this->update(); });
}

void AnalogInput::update() {
  this->emit(output_scale_ * analog_reader_->read());
}

bool AnalogInput::to_json(JsonObject& root) {
  root["read_delay"] = read_delay_;
  return true;
};

bool AnalogInput::from_json(const JsonObject& config) {
  String const expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config[str].is<JsonVariant>()) {
      return false;
    }
  }
  read_delay_ = config["read_delay"];
  return true;
}

const String ConfigSchema(AnalogInput& obj) {
  return R"###({"type":"object","properties":{"read_delay":{"title":"Read delay","type":"number","description":"Number of milliseconds between each reading of the analog input"}}  })###";
}

}  // namespace sensesp
