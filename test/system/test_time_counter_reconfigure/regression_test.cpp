#include <Arduino.h>
#include <ArduinoJson.h>
#include <unity.h>

#include "sensesp/transforms/time_counter.h"

using namespace sensesp;

class TestCounter : public TimeCounter<bool> {
 public:
  TestCounter() : TimeCounter<bool>("") {}
  void simulate_running_minute() {
    initialized_ = true;
    previous_state_ = true;
    start_time_ms_ = millis64() - 60000;
    duration_at_start_ms_ = 0;
  }
};
void test_edit_running_counter_rebases_elapsed_time() {
  TestCounter counter;
  counter.simulate_running_minute();
  JsonDocument doc;
  doc["duration_s"] = 3600.0;
  auto config = doc.as<JsonObject>();
  TEST_ASSERT_TRUE(counter.from_json(config));
  counter.set(true);
  TEST_ASSERT_FLOAT_WITHIN(1, 3600, counter.get());
  counter.set(false);
  TEST_ASSERT_FLOAT_WITHIN(1, 3600, counter.get());
}
void test_legacy_duration_edit() {
  TestCounter counter;
  counter.simulate_running_minute();
  JsonDocument doc;
  doc["duration"] = 120000;
  auto config = doc.as<JsonObject>();
  TEST_ASSERT_TRUE(counter.from_json(config));
  counter.set(false);
  TEST_ASSERT_FLOAT_WITHIN(1, 120, counter.get());
}
void setUp() {}
void tearDown() {}
void setup() {
  delay(2000);
  UNITY_BEGIN();
  RUN_TEST(test_edit_running_counter_rebases_elapsed_time);
  RUN_TEST(test_legacy_duration_edit);
  UNITY_END();
}
void loop() {}
