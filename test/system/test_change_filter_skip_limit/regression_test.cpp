#include <Arduino.h>
#include <ArduinoJson.h>
#include <unity.h>

#include "sensesp/transforms/change_filter.h"

using namespace sensesp;

void test_emits_after_exact_skip_limit() {
  ChangeFilter filter(10, 100, 2);
  int notifications = 0;
  filter.attach([&]() { ++notifications; });
  filter.set(500);
  TEST_ASSERT_EQUAL_INT(1, notifications);
  filter.set(501);
  filter.set(502);
  TEST_ASSERT_EQUAL_INT(1, notifications);
  filter.set(503);
  TEST_ASSERT_EQUAL_INT(2, notifications);
  TEST_ASSERT_FLOAT_WITHIN(0.001, 503, filter.get());
}
void test_zero_skips_passes_every_value() {
  ChangeFilter filter(10, 100, 0);
  int notifications = 0;
  filter.attach([&]() { ++notifications; });
  filter.set(500);
  filter.set(501);
  TEST_ASSERT_EQUAL_INT(2, notifications);
}
void setUp() {}
void tearDown() {}
void setup() {
  delay(2000);
  UNITY_BEGIN();
  RUN_TEST(test_emits_after_exact_skip_limit);
  RUN_TEST(test_zero_skips_passes_every_value);
  UNITY_END();
}
void loop() {}
