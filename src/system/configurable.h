#ifndef _configurable_H_
#define _configurable_H_

#include <map>

#include "Arduino.h"
#include <ArduinoJson.h>


class Configurable {
 public:
  Configurable(String id, String schema);
  const String id;
  virtual JsonObject& get_configuration(JsonBuffer& buf);
  virtual void set_configuration(JsonObject& config);
  String get_config_schema();
 protected:
  virtual void load_configuration();
  virtual void save_configuration();
 private:
  String schema;
};

extern std::map<String, Configurable*> configurables;

#endif
