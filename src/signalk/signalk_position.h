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


template<>
String SKOutput<Position>::as_signalK();


class SKOutputPosition : public SKOutput<Position> {
 public:
  SKOutputPosition(String sk_path, String config_path="") :
    SKOutput<Position>{sk_path, config_path } {
  }

  virtual String as_signalK() override;
};

#endif
