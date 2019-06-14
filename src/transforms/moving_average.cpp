#include "moving_average.h"

// MovingAverage

MovingAverage::MovingAverage(int n, float k, String config_path) :
    NumericTransform(config_path),
      n{ n },
      k{ k } {
  buf.resize(n, 0);
  load_configuration();
}

void MovingAverage::set_input(float input, uint8_t inputChannel) {
  output += -k*buf[ptr]/n;
  buf[ptr] = input;
  ptr = (ptr+1) % n;
  output += k * input/n;
  notify();
}


JsonObject& MovingAverage::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["k"] = k;
  root["n"] = n;
  root["value"] = output;
  return root;
}

static const char SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "n": { "title": "Number of samples in average", "type": "integer" }
        "k": { "title": "Multiplier", "type": "number" }
    }
  })";

String MovingAverage::get_config_schema() {
  return FPSTR(SCHEMA);
}


bool MovingAverage::set_configuration(const JsonObject& config) {
  String expected[] = {"k", "n"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  k = config["k"];
  int n_new = config["n"];
  // need to reset the ring buffer if size changes
  if (n != n_new) {
    buf.assign(n, 0);
    ptr = 0;
    output = 0;
  }
  return true;
}
