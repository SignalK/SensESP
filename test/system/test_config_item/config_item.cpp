#include "sensesp.h"

#include "sensesp/ui/config_item.h"

#include "sensesp/system/filesystem.h"
#include "sensesp/system/observablevalue.h"
#include "sensesp/system/serializable.h"
#include "sensesp/system/valueproducer.h"
#include "sensesp/transforms/angle_correction.h"
#include "sensesp_base_app.h"
#include "sensesp_minimal_app_builder.h"
#include "unity.h"

using namespace sensesp;

void test_persisting_observable_value_schema() {
  SensESPMinimalAppBuilder builder;
  SensESPMinimalApp* sensesp_app = builder.get_app();
  TEST_ASSERT_NOT_NULL(sensesp_app);

  PersistingObservableValue<int> value{2, "/test"};

  auto config_item = ConfigItem(&value);

  value.set(3);

  String ref_schema = R"###({"type":"object","properties":{"value":{"title":"Value","type":"number"}}})###";
  String schema = config_item->get_config_schema();
  ESP_LOGD("test_schema", "schema: %s", schema.c_str());

  TEST_ASSERT_EQUAL_STRING(ref_schema.c_str(), schema.c_str());

  value.save();
  value.set(4);
  value.load();

  TEST_ASSERT_EQUAL(3, value.get());
}

void setup() {
  esp_log_level_set("*", ESP_LOG_VERBOSE);

  UNITY_BEGIN();

  RUN_TEST(test_persisting_observable_value_schema);

  UNITY_END();
}

void loop() {}
