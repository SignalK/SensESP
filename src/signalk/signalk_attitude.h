/** @file signalk_attitude.h
 *  @brief Vessel Attitude (orientation) data structure definition file
 */

#ifndef _signalk_attitude_H_
#define _signalk_attitude_H_

#include "system/valueproducer.h"

/**
 * An out-of-band signal is needed to pass the information from the
 * Producer (sensor) to the Consumer (SignalKOutput) whether the data
 * is valid or not. Using magic numbers can be done (e.g. a negative
 * value for temperature or heading) but needs customization for
 * particular parameters and data types. Rather than special values,
 * for Attitude we add a field to the struct that indicates
 * whether or not the numerical members are valid.
 */
struct Attitude {
  bool is_data_valid;  ///< Indicates whether yaw,pitch,roll data are valid.
  float yaw;           ///< Compass heading of the vessel in radians.
  float pitch;         ///< Rotation about transverse axis in radians. Bow up is
                       ///< positive.
  float roll;  ///< Rotation about longitudinal axis in radians. Starboard roll
               ///< is positive.
};

typedef ValueProducer<Attitude> AttitudeProducer;  // better in valueproducer.h?

#endif  // _signalk_attitude_H_
