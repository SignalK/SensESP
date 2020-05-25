#include "median.h"


Median::Median(unsigned int sampleSize, String config_path) :
    NumericTransform(config_path),
    sampleSize{sampleSize} {
  className = "Median";
  load_configuration();
  buf.reserve(sampleSize);
  buf.clear();
}

void Median::set_input(float input, uint8_t inputChannel) {

  buf.push_back(input);
  if (buf.size() >= sampleSize) {
    // Its time to output a value
    sort(buf.begin(), buf.end());
    int mid = sampleSize / 2;
    output = buf[mid];
    buf.clear();
    notify();
  }
}


JsonObject& Median::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["sampleSize"] = sampleSize;
  return root;
}

static const char SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "sampleSize": { "title": "Sample size", "description": "Number of samples to take before outputing a value", "type": "integer" }
    }
  })";

String Median::get_config_schema() {
  return FPSTR(SCHEMA);
}


bool Median::set_configuration(const JsonObject& config) {
  String expected[] = {"sampleSize"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  unsigned int sampleSize_new = config["sampleSize"];
  if (sampleSize != sampleSize_new) {
    sampleSize = sampleSize_new;
    buf.reserve(sampleSize);
    buf.clear();
  }
  return true;
}
