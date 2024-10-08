#include "sensesp.h"

#include "sensesp/system/serializable.h"
#include "sensesp/system/valueproducer.h"
#include "sensesp/transforms/angle_correction.h"
#include "sensesp_base_app.h"
#include "sensesp_minimal_app_builder.h"
#include "unity.h"

using namespace sensesp;

class TestProducer : public ValueProducer<int>, public Serializable {
 public:
  TestProducer() : ValueProducer<int>() {}

  bool to_json(JsonObject& root) override {
    root["a"] = a_;
    root["b"] = b_;
    JsonArray float_array = root["c"].to<JsonArray>();
    for (int i = 0; i < 3; i++) {
      float_array.add(c_[i]);
    }
    JsonObject map = root["map"].to<JsonObject>();
    map[map_key_] = map_value_;
    return true;
  }

  // Normally these would be protected, but for testing purposes, we make them
  // public.
  int a_ = 567;
  String b_ = "hello";
  float c_[3] = {1.1, 2.2, 3.3};
  String map_key_ = "key";
  String map_value_ = "value";

  bool from_json(const JsonObject& root) override {
    a_ = root["a"];
    b_ = root["b"].as<String>();
    JsonArray float_array = root["c"];
    for (int i = 0; i < 3; i++) {
      c_[i] = float_array[i];
    }
    JsonObject map = root["map"];
    map_value_ = map["key"].as<String>();
    return true;
  }
};

void test_to_json() {
  TestProducer producer;
  JsonDocument doc;
  JsonObject root = doc.to<JsonObject>();

  // This is not particularly useful as a test because we are simply testing
  // the implementation written above.

  producer.to_json(root);

  TEST_ASSERT_EQUAL(567, root["a"]);
  TEST_ASSERT_EQUAL_STRING("hello", root["b"]);
  TEST_ASSERT_EQUAL_FLOAT(1.1, root["c"][0]);
  TEST_ASSERT_EQUAL_FLOAT(2.2, root["c"][1]);
  TEST_ASSERT_EQUAL_FLOAT(3.3, root["c"][2]);
  TEST_ASSERT_EQUAL_STRING("value", root["map"]["key"]);

  String json_str;
  serializeJson(doc, json_str);
  ESP_LOGD("test_to_json", "json_str: %s", json_str.c_str());

  String test_str =
      R"...({"a":567,"b":"hello","c":[1.1,2.2,3.3],"map":{"key":"value"}})...";

  TEST_ASSERT_EQUAL_STRING(test_str.c_str(), json_str.c_str());
}

void test_from_json() {
  String test_str =
      R"...({"a":123,"b":"goodbye","c":[4.4,5.5,6.6],"map":{"key":"new_value"}})...";

  TestProducer producer;
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, test_str);
  TEST_ASSERT_FALSE(error);

  JsonObject root = doc.as<JsonObject>();
  producer.from_json(root);

  TEST_ASSERT_EQUAL(123, producer.a_);
  TEST_ASSERT_EQUAL_STRING("goodbye", producer.b_.c_str());
  TEST_ASSERT_EQUAL_FLOAT(4.4, producer.c_[0]);
  TEST_ASSERT_EQUAL_FLOAT(5.5, producer.c_[1]);
  TEST_ASSERT_EQUAL_FLOAT(6.6, producer.c_[2]);
  TEST_ASSERT_EQUAL_STRING("new_value", producer.map_value_.c_str());
}

// To access protected members, we need to create a child class
class AngleCorrection_ : public AngleCorrection {
 public:
  AngleCorrection_(float offset, float min_angle, const String& config_path = "")
      : AngleCorrection(offset, min_angle, config_path) {}

  float get_offset() { return offset_; }
  float get_min_angle() { return min_angle_; }
};

void test_angle_correction_to_json() {
  AngleCorrection_ angle_correction(1.1, 2.2);
  JsonDocument doc;
  JsonObject root = doc.to<JsonObject>();

  angle_correction.to_json(root);

  TEST_ASSERT_EQUAL_FLOAT(1.1, root["offset"]);
  TEST_ASSERT_EQUAL_FLOAT(2.2, root["min_angle"]);
}

void test_angle_correction_from_json() {
  String test_str = R"...({"offset":3.3,"min_angle":4.4})...";

  AngleCorrection_ angle_correction(1.1, 2.2);
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, test_str);
  TEST_ASSERT_FALSE(error);

  JsonObject root = doc.as<JsonObject>();
  angle_correction.from_json(root);

  TEST_ASSERT_EQUAL_FLOAT(3.3, angle_correction.get_offset());
  TEST_ASSERT_EQUAL_FLOAT(4.4, angle_correction.get_min_angle());
}

void setup() {
  esp_log_level_set("*", ESP_LOG_VERBOSE);

  UNITY_BEGIN();

  RUN_TEST(test_to_json);
  RUN_TEST(test_from_json);
  RUN_TEST(test_angle_correction_to_json);
  RUN_TEST(test_angle_correction_from_json);

  UNITY_END();
}

void loop() {}
