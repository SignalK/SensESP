/**
 * @file constant_sensor.cpp
 * @brief Example of a ConstantSensor that outputs a constant value at given
 *   time intervals.
 *
 * ConstantSensor is a "sensor" that outputs a constant value at given time
 * intervals. The interval, output value and the path can be set in the
 * user interface.
 *
 * This example outputs a fresh water tank capacity of 100 litres (0.1 m3) every
 * 10 seconds.
 */

#include "sensesp/sensors/constant_sensor.h"

#include "sensesp_app_builder.h"

using namespace sensesp;

void setup() {
  SetupLogging();

  // Create the builder object
  SensESPAppBuilder builder;
  sensesp_app = builder.get_app();

  // Create the ConstantSensor object
  auto* constant_sensor =
      new FloatConstantSensor(0.1, 10, "/Sensors/Frash Water Tank Capacity");

  // create and connect the output object

  constant_sensor->connect_to(
      new SKOutputFloat("tanks.freshWater.capacity", "",
                        new SKMetadata("m3", "Fresh Water Tank Capacity")));
}

void loop() {
  event_loop()->tick();
}
