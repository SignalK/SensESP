#include "sensesp.h"

#include "configurable.h"

#ifdef ESP8266
#include "FS.h"
#elif defined(ESP32)
#include "SPIFFS.h"
#endif

// Define a global configurable map. Rationale for a global variable:
// Every Configurable with an id gets registered, and carrying an object
// reference around would unnecessarily reduce readability of the code.
std::map<String, Configurable*> configurables;

Configurable::Configurable(String config_path="")
    : config_path{config_path} {
  if (config_path != "") {
    auto it = configurables.find(config_path);
    if (it != configurables.end()) {
      debugW("WARNING: Overriding id %s", config_path.c_str());
    }
    configurables[config_path] = this;
  }
}

JsonObject& Configurable::get_configuration(JsonBuffer& buf) {
  debugW("WARNING: get_configuration not defined");
  return buf.createObject();
}

// Sets and saves the configuration
bool Configurable::set_configuration(const JsonObject& config) {
  debugW("WARNING: set_configuration not defined");
  return false;
}


String Configurable::get_config_schema() {
  return "{}";
}

void Configurable::load_configuration() {
  if (config_path=="") {
    debugI(
      "Not loading configuration: no config_path specified: %s",
      config_path.c_str());
    return;
  }
  if (!SPIFFS.exists(config_path)) {
    debugI(
      "Not loading configuration: file does not exist: %s",
      config_path.c_str());
    return;
  }

  File f = SPIFFS.open(config_path, "r");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& obj = jsonBuffer.parse(f);
  if (!obj.success()) {
    debugW(
      "WARNING: Could not parse configuration for %s",
      config_path.c_str());
  }
  if (!set_configuration(obj)) {
    debugW(
      "WARNING: Could not set configuration for %s",
      config_path.c_str());
  }
  f.close();
}

void Configurable::save_configuration() {
  if (config_path=="") {
    debugI("WARNING: Could not save configuration (config_path not set)");
  }
  DynamicJsonBuffer jsonBuffer;
  JsonObject& obj = get_configuration(jsonBuffer);
  File f = SPIFFS.open(config_path, "w");
  obj.printTo(f);
  f.close();
}
