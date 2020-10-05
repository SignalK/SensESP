#include "median.h"

Median::Median(unsigned int sample_size, String config_path)
    : NumericTransform(config_path), sample_size{sample_size} {
  load_configuration();
  buf.reserve(sample_size);
  buf.clear();
}

void Median::set_input(float input, uint8_t inputChannel) {
  buf.push_back(input);
  if (buf.size() >= sample_size) {
    // Its time to output a value
    sort(buf.begin(), buf.end());
    int mid = sample_size / 2;
    output = buf[mid];
    buf.clear();
    notify();
  }
}

void Median::get_configuration(JsonObject& root) {
  root["sample_size"] = sample_size;
}

static const char SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "sample_size": { "title": "Sample size", "description": "Number of samples to take before outputing a value", "type": "integer" }
    }
  })";

String Median::get_config_schema() { return FPSTR(SCHEMA); }

bool Median::set_configuration(const JsonObject& config) {
  String expected[] = {"sample_size"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  unsigned int sample_size_new = config["sample_size"];
  if (sample_size != sample_size_new) {
    sample_size = sample_size_new;
    buf.reserve(sample_size);
    buf.clear();
  }
  return true;
}
