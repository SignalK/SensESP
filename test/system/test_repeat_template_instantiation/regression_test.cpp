#include <Arduino.h>
#include <unity.h>

#include "sensesp/transforms/repeat.h"
#include "sensesp_minimal_app_builder.h"

using namespace sensesp;

void test_repeat_variants_accept_values() {
  RepeatStopping<int> stopping(10, 1000);
  stopping.set(5);
  TEST_ASSERT_EQUAL_INT(5, stopping.get());
  RepeatReport<int> report(10);
  report.set(7);
  TEST_ASSERT_EQUAL_INT(7, report.get());
  RepeatConstantRate<float> constant(10, 1000);
  int notifications = 0;
  constant.attach([&]() { ++notifications; });
  const float value = 3.0;
  constant.set(value);
  TEST_ASSERT_EQUAL_INT(0, notifications);
  delay(15);
  event_loop()->tick();
  TEST_ASSERT_GREATER_THAN_INT(0, notifications);
  TEST_ASSERT_FLOAT_WITHIN(0.001, 3, constant.get().value());
}
void setUp() {}
void tearDown() {}
void setup() {
  delay(2000);
  SensESPMinimalAppBuilder builder;
  auto app = builder.get_app();
  UNITY_BEGIN();
  RUN_TEST(test_repeat_variants_accept_values);
  UNITY_END();
}
void loop() {}
