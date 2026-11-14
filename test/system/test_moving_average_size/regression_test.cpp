#include <Arduino.h>
#include <ArduinoJson.h>
#include <unity.h>

#include "sensesp/transforms/moving_average.h"

using namespace sensesp;

void test_invalid_config_preserves_average() {
  MovingAverage average(2, 2);
  average.set(10);
  for (int n : {0, -1}) {
    JsonDocument doc;
    doc["sample_size"] = n;
    doc["multiplier"] = 99;
    auto config = doc.as<JsonObject>();
    TEST_ASSERT_FALSE(average.from_json(config));
  }
  average.set(20);
  TEST_ASSERT_FLOAT_WITHIN(0.001, 30, average.get());
}
void test_invalid_constructor_uses_one_sample() {
  for (int n : {0, -1}) {
    MovingAverage average(n);
    average.set(5);
    average.set(7);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 7, average.get());
  }
}
void setUp() {}
void tearDown() {}
void setup() {
  delay(2000);
  UNITY_BEGIN();
  RUN_TEST(test_invalid_config_preserves_average);
  RUN_TEST(test_invalid_constructor_uses_one_sample);
  UNITY_END();
}
void loop() {}
