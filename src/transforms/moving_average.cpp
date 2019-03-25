#include "moving_average.h"

// MovingAverage

MovingAverage::MovingAverage(String path, float k, int n, String id, String schema)
    : Transform{ path, id, schema },
      k{ k },
      n{ n } {
  buf.resize(n, 0);
  ptr = 0;
  load_configuration();
}

void MovingAverage::set_input(float input) {
  output += -k*buf[ptr]/n;
  buf[ptr] = input;
  ptr = (ptr+1) % n;
  output += k * input/n;
  notify();
}

String MovingAverage::as_json() {
  DynamicJsonBuffer jsonBuffer;
  String json;
  JsonObject& root = jsonBuffer.createObject();
  root.set("path", this->sk_path);
  root.set("value", output);
  root.printTo(json);
  return json;
}

JsonObject& MovingAverage::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["k"] = k;
  root["n"] = n;
  root["sk_path"] = sk_path;
  root["value"] = output;
  return root;
}

bool MovingAverage::set_configuration(const JsonObject& config) {
  String expected[] = {"k", "sk_path"};
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
  sk_path = config["sk_path"].as<String>();
  return true;
}

