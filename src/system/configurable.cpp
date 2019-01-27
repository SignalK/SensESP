#include "configurable.h"

// Define a global configurable map. Rationale for a global variable:
// Every Configurable with an id gets registered, and carrying an object
// reference around would unnecessarily reduce readability of the code.
std::map<String, Configurable*> configurables;

Configurable::Configurable(String id="", String schema="")
    : id{id}, schema{schema} {
  if (id != "") {
    configurables[id] = this;
  }
}

// Returns the current configuration
JsonObject& Configurable::get_configuration(JsonBuffer& buf) {
  Serial.println(F("WARNING: get_configuration not defined"));
  return buf.createObject();
}

// Sets and saves the configuration
void Configurable::set_configuration(JsonObject& config) {
  Serial.println(F("WARNING: set_configuration not defined"));
}

// Returns the configuration schema.
// The schema is a Json object defining and describing the configuration
// keys and possibly allowed value types. The schema can be used
// in client code to render a configuration UI with helpful
// descriptions.
// Schema format TBD.
String Configurable::get_config_schema() {
  Serial.println(F("WARNING: get_config_schema not defined"));
}

void Configurable::load_configuration() {
  Serial.println(F("WARNING: load_configuration not defined"));
}

void Configurable::save_configuration() {
  Serial.println(F("WARNING: save_configuration not defined"));
}
