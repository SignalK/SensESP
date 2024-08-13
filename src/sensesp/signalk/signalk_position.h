#ifndef _signalk_position_H_
#define _signalk_position_H_

#include <set>

#include "sensesp/signalk/signalk_output.h"
#include "sensesp/types/position.h"

namespace sensesp {

template <>
String SKOutput<Position>::as_signalk();

typedef SKOutput<Position> SKOutputPosition;

}  // namespace sensesp

#endif
