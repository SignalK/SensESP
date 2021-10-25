#include "analog_input.h"

#include "Arduino.h"
#include "sensesp.h"

AnalogInput::AnalogInput(uint8_t pin, uint read_delay, String config_path,
                         float output_scale)
    : FloatSensor(config_path),
      pin{pin},
      read_delay{read_delay},
      output_scale{output_scale} {
  analog_reader = new AnalogReader(pin);
  load_configuration();
}

void AnalogInput::update() {
  this->emit(output_scale * analog_reader->read());
}

void AnalogInput::start() {
  if (this->analog_reader->configure()) {
    app.onRepeat(read_delay, [this]() { this->update(); });
  }
}

void AnalogInput::get_configuration(JsonObject& root) {
  root["read_delay"] = read_delay;
};

static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "read_delay": { "title": "Read delay", "type": "number", "description": "Number of milliseconds between each analogRead(A0)" }
    }
  })###";

String AnalogInput::get_config_schema() { return FPSTR(SCHEMA); }

bool AnalogInput::set_configuration(const JsonObject& config) {
  String expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  read_delay = config["read_delay"];
  return true;
}
