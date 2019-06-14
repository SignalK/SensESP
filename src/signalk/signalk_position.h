#ifndef _signalk_position_H_
#define _signalk_position_H_

#include <set>

#include "Arduino.h"
#include <ArduinoJson.h>

#include "sensors/gps.h"
#include "sensesp.h"
#include "signalk_output.h"

///////////////////
// provide correct output formatting for GNSS position

template<>
String SKOutput<Position>::as_signalK();

typedef SKOutput<Position> SKOutputPosition;

#endif
