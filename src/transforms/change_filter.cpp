#include "change_filter.h"
#include <cmath>


static float absf(float val) {
    if (val < 0) {
        return -val;
    }
    else {
        return val;
    }
}


void ChangeFilter::set_input(float newValue, uint8_t inputChannel) {

    float av = absf(newValue - output);
    if (av >= minDelta) {
        output = newValue;
        notify();
    }
}


JsonObject& ChangeFilter::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["minDelta"] = minDelta;
  return root;
}

static const char SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "minDelta": { "title": "Minimum delta", "description": "Minimum difference in change of value before forwarding", "type": "number" }
    }
  })";


String ChangeFilter::get_config_schema() {
  return FPSTR(SCHEMA);
}


bool ChangeFilter::set_configuration(const JsonObject& config) {
  String expected[] = {"minDelta"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  minDelta = config["minDelta"];
  return true;
}
