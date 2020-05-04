#include "timer.h"

long unsigned int Timer::lastSampleTime =  0;

Timer::Timer(unsigned long resetPeriod, bool default_state, String config_path) :
    Transform<bool, int>(config_path),
    time{ 0 },
    last_value{ true },
    init_time{ 0 },
    end_time{ 0 },
    add{ false },
    resetPeriod{ resetPeriod },
    defaultState{ defaultState} {
  className = "Timer";
  load_configuration();
}

void Timer::set_input(bool input, uint8_t inputChannel) {

   long now = millis();
   long secs = now / 1000;

   if (millis() - lastSampleTime >= resetPeriod) {
     lastSampleTime += resetPeriod;
     time = 0;
   }

  if (defaultState == input && last_value) {
    init_time = secs; 
    last_value = false;
    add = true;
  } else {
      end_time = secs;
      last_value = true;
  }
  if (add && last_value) {
    time += end_time - init_time;
    add = false;
  }
    output = time;
  notify();
}


JsonObject& Timer::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["resetPeriod"] = resetPeriod;
  root["defaultState"] = defaultState;
  root["time"] = output;
  return root;
}

  static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "resetPeriod": { "title": "Reset perid", "type": "number", "description": "The time, in milliseconds, between each reset cycle" },
        "defaultState": { "title": "Default state", "type" : "boolean", "description": "Default state of input" },
        "time": { "title": "Time", "type" : "number" }
    }
  })###";

String Timer::get_config_schema() {
  return FPSTR(SCHEMA);
}


bool Timer::set_configuration(const JsonObject& config) {
  String expected[] = {"resetPeriod", "defaultState", "time"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  resetPeriod = config["resetPeriod"];
  defaultState = config["defaultState"];
  time = config["time"];
  return true;
}
