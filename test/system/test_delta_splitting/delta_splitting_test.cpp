/**
 * @file delta_splitting_test.cpp
 * @brief Tests that SKDeltaQueue splits a batch instead of building one delta
 *        the transport has to drop (SignalK/SensESP#1085).
 *
 * On-target: SKDeltaQueue pulls in the event loop and the app singleton, so it
 * cannot run on the host. The size arithmetic it relies on is covered by
 * test/native/test_delta_packing.
 *
 *   pio test -e pioarduino_esp32 -f "system/test_delta_splitting"
 */

#include <Arduino.h>
#include <ArduinoJson.h>

#include <vector>

#include "sensesp/signalk/signalk_delta_queue.h"
#include "sensesp_minimal_app_builder.h"
#include "unity.h"

using namespace sensesp;

// One queued value, sized so that several of them exceed a small budget.
static String make_item(int index) {
  char buf[96];
  snprintf(buf, sizeof(buf), "{\"path\":\"test.path.%02d\",\"value\":1.2345678}",
           index);
  return String(buf);
}

// Every path the deltas carry, in order, so nothing is lost or reordered.
static std::vector<String> collect_paths(const std::vector<String>& deltas) {
  std::vector<String> paths;
  for (const auto& delta : deltas) {
    JsonDocument doc;
    if (deserializeJson(doc, delta) != DeserializationError::Ok) {
      continue;
    }
    for (JsonObjectConst update : doc["updates"].as<JsonArrayConst>()) {
      for (JsonObjectConst value : update["values"].as<JsonArrayConst>()) {
        paths.push_back(value["path"].as<String>());
      }
    }
  }
  return paths;
}

static size_t longest(const std::vector<String>& deltas) {
  size_t max_length = 0;
  for (const auto& delta : deltas) {
    if (delta.length() > max_length) {
      max_length = delta.length();
    }
  }
  return max_length;
}

// A batch that does not fit in the budget comes back as several deltas, each
// within the budget, carrying every value in order.
void test_batch_splits_and_keeps_every_value() {
  SensESPMinimalAppBuilder builder;
  auto app = builder.get_app();
  TEST_ASSERT_NOT_NULL(app);

  SKDeltaQueue queue{40};
  const int kItems = 20;
  for (int i = 0; i < kItems; i++) {
    queue.append(make_item(i));
  }

  std::vector<String> deltas;
  queue.get_deltas(deltas, 256);

  TEST_ASSERT_GREATER_THAN_UINT(1, deltas.size());
  TEST_ASSERT_LESS_OR_EQUAL_UINT(256, longest(deltas));

  std::vector<String> paths = collect_paths(deltas);
  TEST_ASSERT_EQUAL_UINT(kItems, paths.size());
  for (int i = 0; i < kItems; i++) {
    TEST_ASSERT_EQUAL_STRING(make_item(i).c_str(),
                             String("{\"path\":\"" + paths[i] +
                                    "\",\"value\":1.2345678}")
                                 .c_str());
  }
}

// A budget that fits the whole batch produces exactly one delta, as before.
void test_batch_within_budget_stays_one_delta() {
  SensESPMinimalAppBuilder builder;
  auto app = builder.get_app();

  SKDeltaQueue queue{40};
  for (int i = 0; i < 3; i++) {
    queue.append(make_item(i));
  }

  std::vector<String> deltas;
  queue.get_deltas(deltas, 2048);

  TEST_ASSERT_EQUAL_UINT(1, deltas.size());
  TEST_ASSERT_EQUAL_UINT(3, collect_paths(deltas).size());
}

// No budget means no splitting.
void test_zero_budget_does_not_split() {
  SensESPMinimalAppBuilder builder;
  auto app = builder.get_app();

  SKDeltaQueue queue{40};
  for (int i = 0; i < 20; i++) {
    queue.append(make_item(i));
  }

  std::vector<String> deltas;
  queue.get_deltas(deltas, 0);

  TEST_ASSERT_EQUAL_UINT(1, deltas.size());
  TEST_ASSERT_EQUAL_UINT(20, collect_paths(deltas).size());
}

// A single value longer than the budget cannot be split. It goes out alone,
// and the values around it still travel in deltas that fit.
void test_oversize_single_value_is_isolated() {
  SensESPMinimalAppBuilder builder;
  auto app = builder.get_app();

  SKDeltaQueue queue{40};
  queue.append(make_item(0));
  String huge = "{\"path\":\"test.huge\",\"value\":\"";
  while (huge.length() < 400) {
    huge += "x";
  }
  huge += "\"}";
  queue.append(huge);
  queue.append(make_item(1));

  std::vector<String> deltas;
  queue.get_deltas(deltas, 256);

  TEST_ASSERT_EQUAL_UINT(3, deltas.size());
  std::vector<String> paths = collect_paths(deltas);
  TEST_ASSERT_EQUAL_UINT(3, paths.size());
  TEST_ASSERT_EQUAL_STRING("test.huge", paths[1].c_str());
  TEST_ASSERT_LESS_OR_EQUAL_UINT(256, deltas[0].length());
  TEST_ASSERT_LESS_OR_EQUAL_UINT(256, deltas[2].length());
}

void setup() {
  esp_log_level_set("*", ESP_LOG_WARN);

  UNITY_BEGIN();
  RUN_TEST(test_batch_splits_and_keeps_every_value);
  RUN_TEST(test_batch_within_budget_stays_one_delta);
  RUN_TEST(test_zero_budget_does_not_split);
  RUN_TEST(test_oversize_single_value_is_isolated);
  UNITY_END();
}

void loop() {}
