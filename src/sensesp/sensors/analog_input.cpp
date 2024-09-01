#include "sensesp.h"

#include "analog_input.h"

#include "Arduino.h"

namespace sensesp {

AnalogInput::AnalogInput(uint8_t pin, unsigned int read_delay,
                         const String& config_path, float output_scale)
    : FloatSensor(config_path),
      pin{pin},
      read_delay{read_delay},
      output_scale{output_scale} {
  this->set_requires_restart(true);
  analog_reader = new AnalogReader(pin);
  load_configuration();

  if (this->analog_reader->configure()) {
    reactesp::EventLoop::app->onRepeat(read_delay, [this]() { this->update(); });
  }
}

void AnalogInput::update() { this->emit(output_scale * analog_reader->read()); }

void AnalogInput::get_configuration(JsonObject& root) {
  root["read_delay"] = read_delay;
};

static const char kSchema[] = R"###({
    "type": "object",
    "properties": {
        "read_delay": { "title": "Read delay", "type": "number", "description": "Number of milliseconds between each analogRead(A0)" }
    }
  })###";

String AnalogInput::get_config_schema() { return kSchema; }

bool AnalogInput::set_configuration(const JsonObject& config) {
  String const expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  read_delay = config["read_delay"];
  return true;
}

}  // namespace sensesp
