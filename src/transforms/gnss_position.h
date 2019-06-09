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

class GNSSPosition : public SymmetricTransform<Position> {
 public:
  GNSSPosition(String sk_path, String config_path="") :
    SymmetricTransform<Position>{sk_path, config_path} {
  }

  virtual void set_input(Position newValue, uint8_t inputChannel = 0) {
     output = newValue;
     notify();
  }

  virtual String as_signalK() override final;

  virtual JsonObject& get_configuration(JsonBuffer& buf) override final;

  virtual bool set_configuration(const JsonObject& config) override final;
  virtual String get_config_schema() override;

};

#endif
