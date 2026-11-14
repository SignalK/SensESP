#include <Arduino.h>
#include <ArduinoJson.h>
#include <unity.h>

#include "sensesp/transforms/frequency.h"

using namespace sensesp;

void test_same_tick_inputs_are_accumulated() {
  Frequency frequency(1);
  int notifications = 0;
  frequency.attach([&]() { ++notifications; });
  frequency.set(0);
  unsigned long start = millis();
  while (millis() == start) {
    delay(1);
  }
  // Start a fresh measurement interval, then submit another count immediately.
  const unsigned long sample_started = millis();
  frequency.set(2);
  int before = notifications;
  frequency.set(3);
  TEST_ASSERT_EQUAL_INT(before, notifications);
  delay(10);
  const unsigned long sample_duration = millis() - sample_started;
  frequency.set(0);
  TEST_ASSERT_TRUE(std::isfinite(frequency.get()));
  TEST_ASSERT_FLOAT_WITHIN(0.5, 3, frequency.get() * sample_duration / 1000.0);
}
void setUp() {}
void tearDown() {}
void setup() {
  delay(2000);
  UNITY_BEGIN();
  RUN_TEST(test_same_tick_inputs_are_accumulated);
  UNITY_END();
}
void loop() {}
