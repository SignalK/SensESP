#include <Arduino.h>
#include <ArduinoJson.h>
#include <unity.h>

#include "sensesp/transforms/curveinterpolator.h"

using namespace sensesp;

void test_single_sample_is_constant() {
  std::set<CurveInterpolator::Sample> samples = {{5, 7}};
  CurveInterpolator curve(&samples);
  for (float x : {-10.0f, 5.0f, 6.0f, 100.0f}) {
    curve.set(x);
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 7, curve.get());
  }
}
void test_two_sample_extrapolation() {
  std::set<CurveInterpolator::Sample> samples = {{0, 2}, {10, 22}};
  CurveInterpolator curve(&samples);
  curve.set(20);
  TEST_ASSERT_FLOAT_WITHIN(0.0001, 42, curve.get());
}
void setUp() {}
void tearDown() {}
void setup() {
  delay(2000);
  UNITY_BEGIN();
  RUN_TEST(test_single_sample_is_constant);
  RUN_TEST(test_two_sample_extrapolation);
  UNITY_END();
}
void loop() {}
