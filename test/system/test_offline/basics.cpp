#include "sensesp.h"

#include <memory>

#include "sensesp/net/networking.h"
#include "sensesp/system/serializable.h"
#include "sensesp/system/valueproducer.h"
#include "sensesp/transforms/angle_correction.h"
#include "sensesp/transforms/linear.h"
#include "sensesp/transforms/typecast.h"
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
  AngleCorrection_(float offset, float min_angle,
                   const String& config_path = "")
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

// PersistingObservableValue saves itself on change
void test_persisting_observable_value_schema() {
  SensESPMinimalAppBuilder builder;
  auto sensesp_app = builder.get_app();
  TEST_ASSERT_NOT_NULL(sensesp_app);

  ESP_LOGD("test_persisting_observable_value_schema", "about to create object");

  PersistingObservableValue<int> value{2, "/test"};

  ESP_LOGD("test_persisting_observable_value_schema", "object created");

  auto config_item = ConfigItem(&value);

  String ref_schema =
      R"###({"type":"object","properties":{"value":{"title":"Value","type":"number"}}})###";
  String schema = config_item->get_config_schema();
  ESP_LOGD("test_schema", "schema: %s", schema.c_str());

  TEST_ASSERT_EQUAL_STRING(ref_schema.c_str(), schema.c_str());

  // Test manual serialization
  value.set(2);
  JsonDocument doc0;
  JsonObject root0 = doc0.to<JsonObject>();
  bool result = value.to_json(root0);
  TEST_ASSERT_TRUE(result);
  String str0;
  serializeJson(doc0, str0);
  ESP_LOGD("test_schema", "str0: %s", str0.c_str());
  String ref_str0 = R"###({"value":2})###";
  TEST_ASSERT_EQUAL_STRING(ref_str0.c_str(), str0.c_str());

  value.set(0);
  result = value.save();
  TEST_ASSERT_TRUE(result);

  value.set(3);
  result = value.load();
  TEST_ASSERT_TRUE(result);

  TEST_ASSERT_EQUAL(3, value.get());

  value.set(4);
  result = value.load();
  TEST_ASSERT_TRUE(result);

  TEST_ASSERT_EQUAL(4, value.get());

  JsonDocument doc;
  JsonObject root = doc.to<JsonObject>();
  result = value.to_json(root);
  TEST_ASSERT_TRUE(result);

  String json_str;
  serializeJson(doc, json_str);

  String ref_json = R"###({"value":4})###";

  TEST_ASSERT_EQUAL_STRING(ref_json.c_str(), json_str.c_str());

  String json_input = R"###({"value":5})###";

  DeserializationError error = deserializeJson(doc, json_input);
  TEST_ASSERT_TRUE(error == DeserializationError::Ok);

  result = value.from_json(doc.as<JsonObject>());
  TEST_ASSERT_TRUE(result);

  TEST_ASSERT_EQUAL(5, value.get());
}

class Networking_ : public Networking {
 public:
  Networking_(const String& config_path, const String& client_ssid,
              const String& client_password, const String& ap_ssid,
              const String& ap_password)
      : Networking(config_path, client_ssid, client_password, ap_ssid,
                   ap_password) {}

 protected:
  friend void test_save_networking();
};

void test_save_networking() {
  SensESPMinimalAppBuilder builder;
  auto sensesp_app = std::shared_ptr<SensESPMinimalApp>(builder.get_app());
  TEST_ASSERT_NOT_NULL(sensesp_app);

  String wifi_ssid = "Hat Labs Sensors";
  String wifi_password = "fooba";
  String ap_ssid = "SensESP AP!";
  String ap_password = "thisisexcellent";

  auto networking_ptr = new Networking_("/System/WiFi Settings", wifi_ssid,
                                        wifi_password, ap_ssid, ap_password);

  networking_ptr->clear();

  delete networking_ptr;

  auto networking = std::make_shared<Networking_>(
      "/System/WiFi Settings", wifi_ssid, wifi_password, ap_ssid, ap_password);

  networking->save();

  networking->load();

  TEST_ASSERT_EQUAL_STRING(wifi_ssid.c_str(),
                           networking->client_settings_[0].ssid_.c_str());
  TEST_ASSERT_EQUAL_STRING(wifi_password.c_str(),
                           networking->client_settings_[0].password_.c_str());
  TEST_ASSERT_EQUAL_STRING(ap_ssid.c_str(),
                           networking->ap_settings_.ssid_.c_str());
  TEST_ASSERT_EQUAL_STRING(ap_password.c_str(),
                           networking->ap_settings_.password_.c_str());
}

void test_connect_to() {
  SensESPMinimalAppBuilder builder;
  auto sensesp_app = std::shared_ptr<SensESPMinimalApp>(builder.get_app());
  TEST_ASSERT_NOT_NULL(sensesp_app);

  ObservableValue<int> value{2};
  ObservableValue<int> value2;
  ObservableValue<float> float_value;

  // Same types

  value.connect_to(value2);
  value.set(3);
  TEST_ASSERT_EQUAL(3, value2.get());

  // Same types, pointers (using smart pointers for automatic cleanup)

  auto value3 = std::make_shared<ObservableValue<int>>(4);
  auto value4 = std::make_shared<ObservableValue<int>>();
  value3.get()->connect_to(value4.get());
  value3.get()->set(5);
  TEST_ASSERT_EQUAL(5, value4.get()->get());

  // Same types, smart pointers

  auto value5 = std::make_shared<ObservableValue<int>>(6);
  auto value6 = std::make_shared<ObservableValue<int>>();
  value5->connect_to(value6);
  value5->set(7);
  TEST_ASSERT_EQUAL(7, value6->get());

  // Automatic type conversion

  value.connect_to(float_value);
  value.set(4);
  TEST_ASSERT_EQUAL(4, value2.get());
  TEST_ASSERT_EQUAL(4, float_value.get());

  // Automatic type conversion, pointers

  auto value7 = std::make_shared<ObservableValue<int>>(8);
  auto value8 = std::make_shared<ObservableValue<float>>();
  value7.get()->connect_to(value8.get());
  value7.get()->set(9);
  TEST_ASSERT_EQUAL(9, value8.get()->get());

  // Automatic type conversion, smart pointers

  auto value9 = std::make_shared<ObservableValue<int>>(10);
  auto value10 = std::make_shared<ObservableValue<float>>();
  value9->connect_to(value10);
  value9->set(11);
  TEST_ASSERT_EQUAL(11, value10->get());

  // Chaining of same types

  ObservableValue<float> value11{1};
  Linear linear12{2.0, 3.0};
  ObservableValue<float> value13;
  value11.connect_to(linear12)->connect_to(value13);
  value11.set(2);
  TEST_ASSERT_EQUAL(7.0, value13.get());

  // Chaining of same types, pointers

  auto value14 = std::make_shared<ObservableValue<float>>(1);
  auto linear15 = std::make_shared<Linear>(2.0, 3.0);
  auto value16 = std::make_shared<ObservableValue<float>>();
  value14.get()->connect_to(linear15.get())->connect_to(value16.get());
  value14.get()->set(2);
  TEST_ASSERT_EQUAL(7.0, value16.get()->get());

  // Chaining of same types, smart pointers

  auto value17 = std::make_shared<ObservableValue<float>>(1);
  auto linear18 = std::make_shared<Linear>(2.0, 3.0);
  auto value19 = std::make_shared<ObservableValue<float>>();
  value17->connect_to(linear18)->connect_to(value19);
  value17->set(2);
  TEST_ASSERT_EQUAL(7.0, value19->get());

  // Chaining with a transform that changes the type

  ObservableValue<float> value20{1.0};
  RoundToInt round21;
  ObservableValue<int> value22;
  value20.connect_to(round21)->connect_to(value22);
  value20.set(2.3);
  TEST_ASSERT_EQUAL(2, value22.get());

  // Chaining with a transform that changes the type, pointers

  auto value23 = std::make_shared<ObservableValue<float>>(1.0);
  auto round24 = std::make_shared<RoundToInt>();
  auto value25 = std::make_shared<ObservableValue<int>>();
  value23.get()->connect_to(round24.get())->connect_to(value25.get());
  value23.get()->set(2.3);
  TEST_ASSERT_EQUAL(2, value25.get()->get());

  // Chaining with a transform that changes the type, smart pointers

  auto value26 = std::make_shared<ObservableValue<float>>(1.0);
  auto round27 = std::make_shared<RoundToInt>();
  auto value28 = std::make_shared<ObservableValue<int>>();
  value26->connect_to(round27)->connect_to(value28);
  value26->set(2.3);
  TEST_ASSERT_EQUAL(2, value28->get());
}

// Now that connections can be made with weak_ptrs, we should be able to
// safely delete both the producer and the consumer.
void test_weak_ptr_connect_to() {
  SensESPMinimalAppBuilder builder;
  auto sensesp_app = builder.get_app();
  TEST_ASSERT_NOT_NULL(sensesp_app);

  auto value = std::make_shared<ObservableValue<int>>(2);
  auto value2 = std::make_shared<ObservableValue<int>>();

  value->connect_to(value2);

  value->set(3);

  TEST_ASSERT_EQUAL(3, value2->get());

  // This will crash if the weak_ptr is not handled correctly
  value->set(4);

  TEST_ASSERT_EQUAL(4, value->get());
}

void setup() {
  esp_log_level_set("*", ESP_LOG_VERBOSE);

  UNITY_BEGIN();

  RUN_TEST(test_to_json);
  RUN_TEST(test_from_json);
  RUN_TEST(test_angle_correction_to_json);
  RUN_TEST(test_angle_correction_from_json);

  RUN_TEST(test_persisting_observable_value_schema);

  RUN_TEST(test_save_networking);

  RUN_TEST(test_connect_to);
  RUN_TEST(test_weak_ptr_connect_to);

  UNITY_END();
}

void loop() {}
