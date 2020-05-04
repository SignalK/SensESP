#include "counter.h"

long unsigned int Counter::lastSampleTime = 0;

Counter::Counter(unsigned long resetPeriod, bool defaultState, String config_path) :
    Transform<bool, int>(config_path),
    count{ 0 },
    last_value{ true },
    resetPeriod{ resetPeriod },
    defaultState{ defaultState} {
  className = "Counter";
  load_configuration();
}

void Counter::set_input(bool input, uint8_t inputChannel) {
   if (millis() - lastSampleTime >= resetPeriod) {
     lastSampleTime += resetPeriod;
     count = 0;
   }

  if (defaultState == input && true == last_value) {
    count++; 
    last_value = false;
  } else {
      last_value = true;
  }
  output = count;
  notify();
}


JsonObject& Counter::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["resetPeriod"] = resetPeriod;
  root["defaultState"] = defaultState;
  root["count"] = output;
  return root;
}

  static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "resetPeriod": { "title": "Reset perid", "type": "number", "description": "The time, in milliseconds, between each reset cycle" },
        "defaultState": { "title": "Default state", "type" : "boolean", "description": "Default state of input" },
        "count": { "title": "Count", "type" : "number" }
    }
  })###";

String Counter::get_config_schema() {
  return FPSTR(SCHEMA);
}


bool Counter::set_configuration(const JsonObject& config) {
  String expected[] = {"resetPeriod", "defaultState", "count"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  resetPeriod = config["resetPeriod"];
  defaultState = config["defaultState"];
  count = config["count"];
  return true;
}
