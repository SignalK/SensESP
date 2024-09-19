#ifndef SENSESP_SRC_SENSESP_SIGNALK_SIGNALK_TYPES_H_
#define SENSESP_SRC_SENSESP_SIGNALK_SIGNALK_TYPES_H_

#include <vector>

#include "sensesp/signalk/signalk_output.h"
#include "sensesp/types/position.h"

namespace sensesp {

template <>
void SKOutput<Position>::as_signalk_json(JsonDocument& doc);

template <>
void SKOutput<ENUVector>::as_signalk_json(JsonDocument& doc);

template <>
void SKOutput<AttitudeVector>::as_signalk_json(JsonDocument& doc);

typedef SKOutput<Position> SKOutputPosition;
typedef SKOutput<ENUVector> SKOutputENUVector;
typedef SKOutput<AttitudeVector> SKOutputAttitudeVector;

}  // namespace sensesp

#endif
