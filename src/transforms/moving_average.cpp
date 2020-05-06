#include "moving_average.h"

// MovingAverage

MovingAverage::MovingAverage(int n, float k, String config_path) :
    NumericTransform(config_path),
      n{ n },
      k{ k } {
  className = "MovingAverage";
  buf.resize(n, 0);
  initialized = false;
  load_configuration();
}

void MovingAverage::set_input(float input, uint8_t inputChannel) {

  // So the first value to be included in the average doesn't default to 0.0
  if (!initialized) {
    buf.assign(n, input);
    output = input;
    initialized = true;
  }
  else {
    // Subtract 1/nth of the oldest value and add 1/nth of the newest value
    output += -k*buf[ptr]/n;
    output += k * input/n;
    
    // Save the most recent input, then advance to the next storage location.
    // When storage location n is reached, start over again at 0.
    buf[ptr] = input;
    ptr = (ptr+1) % n;
  }
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
        "n": { "title": "Number of samples in average", "type": "integer" },
        "k": { "title": "Multiplier", "type": "number" },
        "value": { "title": "Last value", "type" : "number", "readOnly": true }
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
    initialized = false;
    n = n_new;
  }
  return true;
}
