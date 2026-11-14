#include <Arduino.h>
#include <ArduinoJson.h>
#include <unity.h>

#include "sensesp/transforms/air_density.h"

using namespace sensesp;

void test_saturated_air_at_freezing() {
  AirDensity density;
  density.set(std::make_tuple(273.15f, 1.0f, 101325.0f));
  // Vapor pressure is 610.78 Pa at 0 C, not 6.1078 Pa.
  const double expected =
      ((101325 - 610.78) * 0.0289652 + 610.78 * 0.01801) / (8.31446 * 273.15);
  TEST_ASSERT_FLOAT_WITHIN(0.00001, expected, density.get());
}
void test_dry_air_is_unchanged() {
  AirDensity density;
  density.set(std::make_tuple(303.15f, 0.0f, 101325.0f));
  TEST_ASSERT_FLOAT_WITHIN(0.00001, 101325 * 0.0289652 / (8.31446 * 303.15),
                           density.get());
}
void setUp() {}
void tearDown() {}
void setup() {
  delay(2000);
  UNITY_BEGIN();
  RUN_TEST(test_saturated_air_at_freezing);
  RUN_TEST(test_dry_air_is_unchanged);
  UNITY_END();
}
void loop() {}
