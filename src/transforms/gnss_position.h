#ifndef _gnss_position_H_
#define _gnss_position_H_

#include <set>

#include "Arduino.h"
#include <ArduinoJson.h>

#include "devices/gps.h"
#include "transform.h"
#include "sensesp.h"

///////////////////
// provide correct output formatting for GNSS position

class GNSSPosition : public Transform {
 public:
  GNSSPosition() : Transform{"", "", ""} {}
  GNSSPosition(String sk_path, String id="", String schema="")
    : Transform{sk_path, id, schema} {}
  const Position* get() { return output; }
  void set_input(Position* input) {
    output = input;
    notify();
  }
  String as_json() override final;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override final;
  virtual bool set_configuration(const JsonObject& config) override final;
 private:
  Position* output;
};

#endif
