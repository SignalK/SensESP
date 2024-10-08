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

  ConfigItem(constant_sensor)
      ->set_title("Fresh Water Tank Capacity")
      ->set_description("Fresh water tank capacity in m3.")
      ->set_sort_order(1000);

  // create and connect the output object

  auto constant_sensor_output =
      new SKOutputFloat("tanks.freshWater.capacity", "",
                        new SKMetadata("m3", "Fresh Water Tank Capacity"));

  ConfigItem(constant_sensor_output)
      ->set_title("Fresh Water Tank SK Path")
      ->set_description("Signal K path for the fresh water tank capacity.")
      ->set_sort_order(1100);

  constant_sensor->connect_to(constant_sensor_output);
}

void loop() { event_loop()->tick(); }
