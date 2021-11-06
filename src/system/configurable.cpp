#include "configurable.h"

#include "sensesp.h"

#ifdef ESP8266
#include "FS.h"
#elif defined(ESP32)
#include "SPIFFS.h"
#endif

namespace sensesp {

// Define a global configurable map. Rationale for a global variable:
// Every Configurable with an id gets registered, and carrying an object
// reference around would unnecessarily reduce readability of the code.
std::map<String, Configurable*> configurables;

Configurable::Configurable(String config_path = "") : config_path{config_path} {
  if (config_path != "") {
    auto it = configurables.find(config_path);
    if (it != configurables.end()) {
      debugW("WARNING: Overriding id %s", config_path.c_str());
    }
    configurables[config_path] = this;
  }
}

void Configurable::get_configuration(JsonObject& doc) {
  debugW("WARNING: get_configuration not defined");
}

// Sets and saves the configuration
bool Configurable::set_configuration(const JsonObject& config) {
  debugW("WARNING: set_configuration not defined for this Class");
  return false;
}

String Configurable::get_config_schema() { return "{}"; }

void Configurable::load_configuration() {
  if (config_path == "") {
    debugI("Not loading configuration: no config_path specified: %s",
           config_path.c_str());
    return;
  }
  if (!SPIFFS.exists(config_path)) {
    debugI("Not loading configuration: file does not exist: %s",
           config_path.c_str());
    return;
  }

  File f = SPIFFS.open(config_path, "r");
  DynamicJsonDocument jsonDoc(
      1024);  // TODO: set the size of ALL DynamicJsonDocuments throughout the
              // project
  auto error = deserializeJson(jsonDoc, f);
  if (error) {
    debugW("WARNING: Could not parse configuration for %s",
           config_path.c_str());
    return;
  }  //
  if (!set_configuration(jsonDoc.as<JsonObject>())) {
    debugW("WARNING: Could not set configuration for %s", config_path.c_str());
  }
  f.close();
}

void Configurable::save_configuration() {
  if (config_path == "") {
    debugI("WARNING: Could not save configuration (config_path not set)");
  }
  DynamicJsonDocument jsonDoc(1024);
  JsonObject obj = jsonDoc.createNestedObject("root");
  get_configuration(obj);
  File f = SPIFFS.open(config_path, "w");
  serializeJson(obj, f);
  f.close();
}

}  // namespace sensesp
