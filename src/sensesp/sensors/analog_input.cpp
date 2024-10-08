#include "sensesp.h"

#include "analog_input.h"

#include "Arduino.h"
#include "sensesp_base_app.h"

namespace sensesp {

AnalogInput::AnalogInput(uint8_t pin, unsigned int read_delay,
                         const String& config_path, float output_scale)
    : FloatSensor(config_path),
      pin{pin},
      read_delay{read_delay},
      output_scale{output_scale} {
  analog_reader = new AnalogReader(pin);
  load();

  if (this->analog_reader->configure()) {
    event_loop()->onRepeat(read_delay, [this]() { this->update(); });
  }
}

void AnalogInput::update() { this->emit(output_scale * analog_reader->read()); }

bool AnalogInput::to_json(JsonObject& root) {
  root["read_delay"] = read_delay;
  return true;
};

bool AnalogInput::from_json(const JsonObject& config) {
  String const expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config[str].is<JsonVariant>()) {
      return false;
    }
  }
  read_delay = config["read_delay"];
  return true;
}

const String ConfigSchema(AnalogInput& obj) {
  return R"###({"type":"object","properties":{"read_delay":{"title":"Read delay","type":"number","description":"Number of milliseconds between each analogRead(A0)"}}  })###";
}

}  // namespace sensesp
