#include "sensesp.h"

#include "elapsedMillis.h"
#include "sensesp/sensors/sensor.h"
#include "sensesp/system/observablevalue.h"
#include "sensesp/system/lambda_consumer.h"
#include "sensesp_minimal_app_builder.h"
#include "unity.h"

using namespace sensesp;

void test_sensesp() {
  SensESPMinimalAppBuilder builder;
  SensESPMinimalApp* sensesp_app = builder.get_app();
  TEST_ASSERT_NOT_NULL(sensesp_app);

  auto sensor = RepeatSensor<int>(10, []() {
    static int value = 42;
    return value++;
  });

  ObservableValue<int>* observable = new ObservableValue<int>();
  sensor.connect_to(observable);

  auto test_consumer = new LambdaConsumer<int>([](int value) {
    static int target_value = 42;
    TEST_ASSERT_EQUAL(target_value, value);
    TEST_ASSERT_TRUE(42 <= target_value && target_value < 47);
    target_value++;
  });

  elapsedMillis elapsed = 0;
  while (elapsed < 100) {
    event_loop()->tick();
  }
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_sensesp);
  UNITY_END();
}

void loop() {}
