#include <Arduino.h>
#include <ArduinoJson.h>
#include <unity.h>

#include "sensesp/system/serializable.h"
#include "sensesp/transforms/voltage_multiplier.h"
#include "sensesp_minimal_app_builder.h"

using namespace sensesp;

void test_restores_saved_resistors() {
  SensESPMinimalAppBuilder builder;
  TEST_ASSERT_NOT_NULL(builder.get_app().get());
  const char* path = "/test/voltage_multiplier_restore";
  {
    VoltageMultiplier seed(3000, 1000, path);
    seed.clear();
    TEST_ASSERT_TRUE(seed.save());
  }
  VoltageMultiplier restored(1000, 1000, path);
  restored.set(2);
  TEST_ASSERT_FLOAT_WITHIN(0.001, 8, restored.get());
  restored.clear();
}
void setUp() {}
void tearDown() {}
void setup() {
  delay(2000);
  UNITY_BEGIN();
  RUN_TEST(test_restores_saved_resistors);
  UNITY_END();
}
void loop() {}
