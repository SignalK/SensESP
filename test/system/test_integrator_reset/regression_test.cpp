#include <Arduino.h>
#include <ArduinoJson.h>
#include <unity.h>

#include "sensesp/transforms/integrator.h"

using namespace sensesp;

void test_reset_updates_output_and_consumers() {
  Accumulator accumulator;
  int notifications = 0;
  accumulator.attach([&]() { ++notifications; });
  accumulator.set(5);
  accumulator.reset();
  TEST_ASSERT_EQUAL_INT(0, accumulator.get());
  TEST_ASSERT_EQUAL_INT(2, notifications);
  accumulator.set(2);
  TEST_ASSERT_EQUAL_INT(2, accumulator.get());
}
void test_scaled_integrator_after_reset() {
  FloatIntegrator integrator(2);
  integrator.set(3);
  integrator.reset();
  TEST_ASSERT_FLOAT_WITHIN(0.001, 0, integrator.get());
  integrator.set(4);
  TEST_ASSERT_FLOAT_WITHIN(0.001, 8, integrator.get());
}
void setUp() {}
void tearDown() {}
void setup() {
  delay(2000);
  UNITY_BEGIN();
  RUN_TEST(test_reset_updates_output_and_consumers);
  RUN_TEST(test_scaled_integrator_after_reset);
  UNITY_END();
}
void loop() {}
