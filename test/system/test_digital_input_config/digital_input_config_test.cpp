#include <Arduino.h>
#include <ArduinoJson.h>
#include "sensesp/sensors/digital_input.h"
#include "sensesp/ui/config_item.h"
#include "sensesp_minimal_app_builder.h"
#include "unity.h"

using namespace sensesp;

void test_digital_input_config_item() {
  SensESPMinimalAppBuilder builder;
  auto app = builder.get_app();
  auto input = std::make_shared<DigitalInputState>(4, INPUT_PULLUP, 100,
                                                  "/test/digital_input");
  auto item = ConfigItem(input);
  item->set_title("Alarm Input");
  JsonDocument schema;
  TEST_ASSERT_TRUE(deserializeJson(schema, item->get_config_schema()) ==
                   DeserializationError::Ok);
  TEST_ASSERT_TRUE(schema["properties"]["read_delay"].is<JsonObject>());
  JsonDocument document;
  auto config = document.to<JsonObject>();
  config["read_delay"] = 250;
  TEST_ASSERT_TRUE(item->from_json(config));
  document.clear();
  config = document.to<JsonObject>();
  TEST_ASSERT_TRUE(item->to_json(config));
  TEST_ASSERT_EQUAL_INT(250, config["read_delay"].as<int>());
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_digital_input_config_item);
  UNITY_END();
}
void loop() {}
