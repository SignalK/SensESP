/**
 * @file wifi_watchdog_config_load_test.cpp
 * @brief On-target test for WiFiWatchdog config persistence and validation.
 *
 * Seeds a persisted timeout, constructs a fresh WiFiWatchdog through the
 * load()-during-construction path, and asserts the persisted value won over
 * the constructor argument. Then checks that from_json rejects a missing key,
 * a non-integer, and out-of-range values without changing the stored timeout,
 * and that the schema carries the maximum.
 *
 * Compile check: pio test -e pioarduino_esp32 -f system/test_wifi_watchdog_config_load
 *   --without-testing --without-uploading
 */

#include <Arduino.h>

#include <ArduinoJson.h>

#include "sensesp.h"
#include "sensesp/system/serializable.h"
#include "sensesp/system/valueproducer.h"
#include "sensesp/system/wifi_watchdog.h"
#include "sensesp_minimal_app_builder.h"
#include "unity.h"

using namespace sensesp;

namespace {

constexpr const char* kConfigPath = "/test/wifi_watchdog_config_load";
constexpr int kSavedTimeoutS = 600;
constexpr int kCtorTimeoutS = 180;

int serialized_timeout(WiFiWatchdog& watchdog) {
  JsonDocument doc;
  JsonObject obj = doc.to<JsonObject>();
  static_cast<Serializable&>(watchdog).to_json(obj);
  return obj["timeout_s"].as<int>();
}

bool apply_json(WiFiWatchdog& watchdog, const char* json) {
  JsonDocument doc;
  deserializeJson(doc, json);
  JsonObject obj = doc.as<JsonObject>();
  return static_cast<Serializable&>(watchdog).from_json(obj);
}

void tick_for(uint32_t ms) {
  const uint32_t start = millis();
  while (static_cast<uint32_t>(millis() - start) < ms) {
    event_loop()->tick();
    delay(5);
  }
}

}  // namespace

void test_watchdog_restores_persisted_timeout() {
  SensESPMinimalAppBuilder builder;
  auto app = builder.get_app();
  TEST_ASSERT_NOT_NULL(app);

  auto producer = std::make_shared<ValueProducer<NetworkState>>();

  {
    WiFiWatchdog cleaner(producer, kSavedTimeoutS, kConfigPath);
    cleaner.clear();
  }
  {
    WiFiWatchdog seeder(producer, kSavedTimeoutS, kConfigPath);
    TEST_ASSERT_TRUE(seeder.save());
  }

  WiFiWatchdog restored(producer, kCtorTimeoutS, kConfigPath);
  TEST_ASSERT_EQUAL_INT(kSavedTimeoutS, serialized_timeout(restored));

  restored.clear();
}

void test_watchdog_rejects_invalid_timeouts() {
  auto producer = std::make_shared<ValueProducer<NetworkState>>();
  WiFiWatchdog watchdog(producer, kCtorTimeoutS, kConfigPath);

  TEST_ASSERT_FALSE(apply_json(watchdog, "{}"));
  TEST_ASSERT_FALSE(apply_json(watchdog, "{\"timeout_s\":\"180\"}"));
  TEST_ASSERT_FALSE(apply_json(watchdog, "{\"timeout_s\":0}"));
  TEST_ASSERT_FALSE(apply_json(watchdog, "{\"timeout_s\":-5}"));
  String too_large = "{\"timeout_s\":" + String(WiFiWatchdog::kMaxTimeoutS + 1) + "}";
  TEST_ASSERT_FALSE(apply_json(watchdog, too_large.c_str()));
  TEST_ASSERT_EQUAL_INT(kCtorTimeoutS, serialized_timeout(watchdog));

  TEST_ASSERT_TRUE(apply_json(watchdog, "{\"timeout_s\":900}"));
  TEST_ASSERT_EQUAL_INT(900, serialized_timeout(watchdog));
}

void test_watchdog_schema_carries_bounds() {
  auto producer = std::make_shared<ValueProducer<NetworkState>>();
  WiFiWatchdog watchdog(producer, kCtorTimeoutS, kConfigPath);

  JsonDocument doc;
  TEST_ASSERT_TRUE(deserializeJson(doc, ConfigSchema(watchdog)) ==
                   DeserializationError::Ok);
  JsonObject field = doc["properties"]["timeout_s"];
  TEST_ASSERT_EQUAL_INT(1, field["minimum"].as<int>());
  TEST_ASSERT_EQUAL_INT(WiFiWatchdog::kMaxTimeoutS, field["maximum"].as<int>());
}

// With a 1 s timeout: connect, drop, let poll() schedule the restart, then
// reconnect inside the 1 s restart delay. If the cancel fails, ESP.restart()
// fires and the test run never completes.
void test_reconnect_cancels_pending_restart() {
  auto producer = std::make_shared<ValueProducer<NetworkState>>();
  WiFiWatchdog watchdog(producer, 1, kConfigPath);
  watchdog.clear();

  producer->emit(WiFiState::kWifiConnectedToAP);
  producer->emit(WiFiState::kWifiDisconnected);
  tick_for(2200);  // restart scheduled at ~2 s, due at ~3 s
  producer->emit(WiFiState::kWifiConnectedToAP);
  tick_for(2000);
  TEST_PASS();
}

// Raising the timeout from the web UI during the 1 s restart delay must
// cancel the restart, since the outage no longer exceeds the timeout.
void test_timeout_raise_cancels_pending_restart() {
  auto producer = std::make_shared<ValueProducer<NetworkState>>();
  WiFiWatchdog watchdog(producer, 1, kConfigPath);
  watchdog.clear();

  producer->emit(WiFiState::kWifiConnectedToAP);
  producer->emit(WiFiState::kWifiDisconnected);
  tick_for(2200);
  TEST_ASSERT_TRUE(apply_json(watchdog, "{\"timeout_s\":600}"));
  tick_for(2000);
  TEST_PASS();
}

void setup() {
  delay(2000);
  UNITY_BEGIN();
  RUN_TEST(test_watchdog_restores_persisted_timeout);
  RUN_TEST(test_watchdog_rejects_invalid_timeouts);
  RUN_TEST(test_watchdog_schema_carries_bounds);
  RUN_TEST(test_reconnect_cancels_pending_restart);
  RUN_TEST(test_timeout_raise_cancels_pending_restart);
  UNITY_END();
}

void loop() {}
