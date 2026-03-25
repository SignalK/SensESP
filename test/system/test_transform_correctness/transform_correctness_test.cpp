/**
 * @file transform_correctness_test.cpp
 * @brief Tests for transform correctness bug fixes.
 *
 * Covers: MovingAverage from_json resize (#874), Hysteresis uninitialized
 * last_value_ (#906), VoltageDivider division by zero (#907).
 */

#include <Arduino.h>

#include "sensesp/system/lambda_consumer.h"
#include "sensesp/transforms/hysteresis.h"
#include "sensesp/transforms/moving_average.h"
#include "sensesp/transforms/voltagedivider.h"
#include "unity.h"

using namespace sensesp;

// ---------------------------------------------------------------------------
// MovingAverage: from_json with changed sample_size resizes correctly (#874)
// ---------------------------------------------------------------------------

void test_moving_average_from_json_resize() {
  MovingAverage ma(4, 1.0);

  // Feed some values to initialize
  ma.set(10.0f);
  ma.set(20.0f);

  // Change sample_size via from_json
  JsonDocument doc;
  doc["multiplier"] = 1.0;
  doc["sample_size"] = 8;
  JsonObject config = doc.as<JsonObject>();

  bool result = ma.from_json(config);
  TEST_ASSERT_TRUE(result);

  // After resize, feeding values should not crash or produce garbage.
  // The buffer should be re-initialized on next set() since initialized_
  // was reset to false.
  float received = -1.0f;
  LambdaConsumer<float> consumer([&received](float v) { received = v; });
  ma.connect_to(&consumer);

  ma.set(5.0f);
  // After re-initialization, all 8 slots filled with 5.0, average = 5.0
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 5.0f, received);
}

// ---------------------------------------------------------------------------
// Hysteresis: first input in dead zone produces valid output (#906)
// ---------------------------------------------------------------------------

void test_hysteresis_first_input_in_band() {
  // Thresholds: lower=10, upper=20, low_output=0, high_output=1
  Hysteresis<float, int> hyst(10.0f, 20.0f, 0, 1);

  int received = -1;
  LambdaConsumer<int> consumer([&received](int v) { received = v; });
  hyst.connect_to(&consumer);

  // Input 15.0 is between thresholds (in the dead zone).
  // With uninitialized last_value_, this was UB. Now it should return
  // low_output (0) since last_value_ is initialized to low_output.
  hyst.set(15.0f);
  TEST_ASSERT_EQUAL(0, received);
}

void test_hysteresis_normal_operation() {
  Hysteresis<float, int> hyst(10.0f, 20.0f, 0, 1);

  int received = -1;
  LambdaConsumer<int> consumer([&received](int v) { received = v; });
  hyst.connect_to(&consumer);

  // Go above upper threshold -> high
  hyst.set(25.0f);
  TEST_ASSERT_EQUAL(1, received);

  // Drop into dead zone -> should stay high
  hyst.set(15.0f);
  TEST_ASSERT_EQUAL(1, received);

  // Drop below lower threshold -> low
  hyst.set(5.0f);
  TEST_ASSERT_EQUAL(0, received);

  // Rise into dead zone -> should stay low
  hyst.set(15.0f);
  TEST_ASSERT_EQUAL(0, received);
}

// ---------------------------------------------------------------------------
// VoltageDividerR1: Vout=0 does not crash (#907)
// ---------------------------------------------------------------------------

void test_voltage_divider_r1_zero_vout() {
  VoltageDividerR1 vd(1000.0f, 3.3f);

  float received = -999.0f;
  bool was_called = false;
  LambdaConsumer<float> consumer([&](float v) {
    received = v;
    was_called = true;
  });
  vd.connect_to(&consumer);

  // Vout = 0 would cause division by zero. Should silently return.
  vd.set(0.0f);
  TEST_ASSERT_FALSE(was_called);
}

// ---------------------------------------------------------------------------
// VoltageDividerR2: Vout=Vin does not crash (#907)
// ---------------------------------------------------------------------------

void test_voltage_divider_r2_vout_equals_vin() {
  VoltageDividerR2 vd(1000.0f, 3.3f);

  bool was_called = false;
  LambdaConsumer<float> consumer([&](float v) { was_called = true; });
  vd.connect_to(&consumer);

  // Vout = Vin would cause division by zero. Should silently return.
  vd.set(3.3f);
  TEST_ASSERT_FALSE(was_called);
}

// ---------------------------------------------------------------------------
// VoltageDividerR1: normal operation still works
// ---------------------------------------------------------------------------

void test_voltage_divider_r1_normal() {
  VoltageDividerR1 vd(1000.0f, 3.3f);

  float received = -1.0f;
  LambdaConsumer<float> consumer([&received](float v) { received = v; });
  vd.connect_to(&consumer);

  // Vout = 1.65V -> R1 = (3.3 - 1.65) * 1000 / 1.65 = 1000 ohms
  vd.set(1.65f);
  TEST_ASSERT_FLOAT_WITHIN(0.1f, 1000.0f, received);
}

// ---------------------------------------------------------------------------
// Test runner
// ---------------------------------------------------------------------------

void setup() {
  delay(2000);

  UNITY_BEGIN();

  RUN_TEST(test_moving_average_from_json_resize);
  RUN_TEST(test_hysteresis_first_input_in_band);
  RUN_TEST(test_hysteresis_normal_operation);
  RUN_TEST(test_voltage_divider_r1_zero_vout);
  RUN_TEST(test_voltage_divider_r2_vout_equals_vin);
  RUN_TEST(test_voltage_divider_r1_normal);

  UNITY_END();
}

void loop() {}
