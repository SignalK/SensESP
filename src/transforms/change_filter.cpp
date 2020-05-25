#include "change_filter.h"


static float absf(float val) {
    if (val < 0) {
        return -val;
    }
    else {
        return val;
    }
}


ChangeFilter::ChangeFilter(float minDelta, float maxDelta, int maxSkips, String config_path) : 
          NumericTransform(config_path),
          minDelta{minDelta},
          maxDelta{maxDelta},
          maxSkips{maxSkips} {

  className = "ChangeFilter";
  load_configuration();
  skips = maxSkips+1;
}


void ChangeFilter::set_input(float newValue, uint8_t inputChannel) {

    float delta = absf(newValue - output);
    if ((delta >= minDelta && delta <= maxDelta) || skips > maxSkips) {
        output = newValue;
        skips = 0;
        notify();
    }
    else {
        skips++;
    }
}


JsonObject& ChangeFilter::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["minDelta"] = minDelta;
  root["maxDelta"] = maxDelta;
  root["maxSkips"] = maxSkips;
  return root;
}

static const char SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "minDelta": { "title": "Minimum delta", "description": "Minimum difference in change of value before forwarding", "type": "number" },
        "maxDelta": { "title": "Maximum delta", "description": "Maximum difference in change of value to allow forwarding", "type": "number" },
        "maxSkips": { "title": "Max skip count", "description": "Maximum number of consecutive filtered values before one is allowed through", "type": "number" }
    }
  })";


String ChangeFilter::get_config_schema() {
  return FPSTR(SCHEMA);
}


bool ChangeFilter::set_configuration(const JsonObject& config) {
  String expected[] = {"minDelta", "maxDelta", "maxSkips" };
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  minDelta = config["minDelta"];
  maxDelta = config["maxDelta"];
  maxSkips = config["maxSkips"];
  skips = maxSkips+1;
  return true;
}
