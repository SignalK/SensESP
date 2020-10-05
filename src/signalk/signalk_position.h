#ifndef _signalk_position_H_
#define _signalk_position_H_

#include <ArduinoJson.h>

#include <set>

#include "Arduino.h"
#include "sensesp.h"
#include "sensors/gps.h"
#include "signalk_output.h"

///////////////////
// provide correct output formatting for GNSS position

template <>
String SKOutput<Position>::as_signalk();

typedef SKOutput<Position> SKOutputPosition;

#endif
