#ifndef _signalk_position_H_
#define _signalk_position_H_

#include <set>

#include "Arduino.h"
#include <ArduinoJson.h>

#include "devices/gps.h"
#include "sensesp.h"
#include "signalk_output.h"

///////////////////
// provide correct output formatting for GNSS position

class SKOutputPosition : public SKOutputBase<Position> {
 public:
  SKOutputPosition(String sk_path, String config_path="") :
    SKOutputBase<Position>{sk_path, config_path } {
  }

  virtual String as_signalK() override;

  virtual JsonObject& get_configuration(JsonBuffer& buf) override;

  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;

};

#endif
