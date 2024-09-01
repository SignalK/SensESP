#ifndef SENSESP_SRC_SENSESP_SIGNALK_SIGNALK_POSITION_H_
#define SENSESP_SRC_SENSESP_SIGNALK_SIGNALK_POSITION_H_

#include <set>

#include "sensesp/signalk/signalk_output.h"
#include "sensesp/types/position.h"

namespace sensesp {

template <>
String SKOutput<Position>::as_signalk();

typedef SKOutput<Position> SKOutputPosition;

}  // namespace sensesp

#endif
