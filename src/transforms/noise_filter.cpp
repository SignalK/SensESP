#include "noise_filter.h"
#include <cmath>


static float absf(float val) {
    if (val < 0) {
        return -val;
    }
    else {
        return val;
    }
}


NoiseFilter::NoiseFilter(float maxDelta, int maxSkips, String config_path) : 
    maxDelta{maxDelta},
    maxSkips{maxSkips},
    NumericTransform(config_path) {
    skips = maxSkips+1;;
}


void NoiseFilter::set_input(float newValue, uint8_t inputChannel) {

    if (absf(newValue - output) <= maxDelta || skips > maxSkips) {
        output = newValue;
        skips = 0;
        notify();
    }
    else {
        skips++;
    }
}


JsonObject& NoiseFilter::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["maxDelta"] = maxDelta;
  root["maxSkips"] = maxSkips;
  return root;
}

static const char SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "maxDelta": { "title": "Maximum delta", "description": "Maximum difference in change of value to allow forwarding", "type": "number" },
        "maxSkips": { "title": "Max skip count", "description": "Maximum number of consecutive filtered values before one is allowed through", "type": "number" }
    }
  })";


String NoiseFilter::get_config_schema() {
  return FPSTR(SCHEMA);
}


bool NoiseFilter::set_configuration(const JsonObject& config) {
  String expected[] = {"maxDelta", "maxSkips"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  maxDelta = config["maxDelta"];
  maxSkips = config["maxSkips"];
  return true;
}
