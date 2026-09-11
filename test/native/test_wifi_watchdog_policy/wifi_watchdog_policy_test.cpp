/**
 * @file wifi_watchdog_policy_test.cpp
 * @brief Host unit tests for the pure WiFi watchdog policy
 * (sensesp/system/wifi_watchdog_policy.h).
 *
 * Runs on the `native` env (no Arduino):
 *   pio test -e native -f native/test_wifi_watchdog_policy
 */

#include <unity.h>

#include <cstdint>

#include "sensesp/system/wifi_watchdog_policy.h"

using namespace sensesp;

constexpr uint32_t kTimeoutMs = 180000;

// Not armed before the first link-up: a device that boots with no reachable
// AP must not restart, however long it stays disconnected.
void test_not_armed_never_restarts(void) {
  WiFiWatchdogPolicy policy;
  policy.set_link_state(WiFiWatchdogLink::kDown, 0);
  TEST_ASSERT_FALSE(policy.restart_due(kTimeoutMs, kTimeoutMs));
  TEST_ASSERT_FALSE(policy.restart_due(100 * kTimeoutMs, kTimeoutMs));
}

// Armed, down for less than the timeout: no restart.
void test_armed_down_below_timeout_no_restart(void) {
  WiFiWatchdogPolicy policy;
  policy.set_link_state(WiFiWatchdogLink::kUp, 1000);
  policy.set_link_state(WiFiWatchdogLink::kDown, 5000);
  TEST_ASSERT_FALSE(policy.restart_due(5000 + kTimeoutMs - 1, kTimeoutMs));
}

// Armed, down for the timeout or longer: restart.
void test_armed_down_at_timeout_restarts(void) {
  WiFiWatchdogPolicy policy;
  policy.set_link_state(WiFiWatchdogLink::kUp, 1000);
  policy.set_link_state(WiFiWatchdogLink::kDown, 5000);
  TEST_ASSERT_TRUE(policy.restart_due(5000 + kTimeoutMs, kTimeoutMs));
  TEST_ASSERT_TRUE(policy.restart_due(5000 + 2 * kTimeoutMs, kTimeoutMs));
}

// Armed while up: never due.
void test_armed_up_never_restarts(void) {
  WiFiWatchdogPolicy policy;
  policy.set_link_state(WiFiWatchdogLink::kUp, 1000);
  TEST_ASSERT_FALSE(policy.restart_due(1000 + 10 * kTimeoutMs, kTimeoutMs));
}

// Down, then up before the timeout: the timer is cleared. A later drop
// counts from the new drop time, not the first one.
void test_reconnect_clears_timer(void) {
  WiFiWatchdogPolicy policy;
  policy.set_link_state(WiFiWatchdogLink::kUp, 1000);
  policy.set_link_state(WiFiWatchdogLink::kDown, 5000);
  policy.set_link_state(WiFiWatchdogLink::kUp, 60000);
  TEST_ASSERT_FALSE(policy.restart_due(5000 + kTimeoutMs, kTimeoutMs));
  policy.set_link_state(WiFiWatchdogLink::kDown, 100000);
  TEST_ASSERT_FALSE(policy.restart_due(100000 + kTimeoutMs - 1, kTimeoutMs));
  TEST_ASSERT_TRUE(policy.restart_due(100000 + kTimeoutMs, kTimeoutMs));
}

// Repeated down reports do not move the start of the outage.
void test_repeated_down_keeps_original_start(void) {
  WiFiWatchdogPolicy policy;
  policy.set_link_state(WiFiWatchdogLink::kUp, 1000);
  policy.set_link_state(WiFiWatchdogLink::kDown, 5000);
  policy.set_link_state(WiFiWatchdogLink::kDown, 5000 + kTimeoutMs / 2);
  TEST_ASSERT_TRUE(policy.restart_due(5000 + kTimeoutMs, kTimeoutMs));
}

// Captive-portal (neutral) state: does not arm, and stops a running timer.
void test_neutral_does_not_arm(void) {
  WiFiWatchdogPolicy policy;
  policy.set_link_state(WiFiWatchdogLink::kNeutral, 0);
  policy.set_link_state(WiFiWatchdogLink::kDown, 1000);
  TEST_ASSERT_FALSE(policy.restart_due(1000 + 10 * kTimeoutMs, kTimeoutMs));
}

void test_neutral_clears_running_timer(void) {
  WiFiWatchdogPolicy policy;
  policy.set_link_state(WiFiWatchdogLink::kUp, 1000);
  policy.set_link_state(WiFiWatchdogLink::kDown, 5000);
  policy.set_link_state(WiFiWatchdogLink::kNeutral, 6000);
  TEST_ASSERT_FALSE(policy.restart_due(5000 + 10 * kTimeoutMs, kTimeoutMs));
}

// millis() wraps at 2^32. A drop shortly before the wrap must fire at the
// right elapsed time after it, and never early.
void test_clock_wraparound(void) {
  WiFiWatchdogPolicy policy;
  const uint32_t down_at = UINT32_MAX - 1000;
  policy.set_link_state(WiFiWatchdogLink::kUp, down_at - 1);
  policy.set_link_state(WiFiWatchdogLink::kDown, down_at);
  const uint32_t due_at = down_at + kTimeoutMs;  // wraps
  TEST_ASSERT_TRUE(due_at < down_at);
  TEST_ASSERT_FALSE(policy.restart_due(due_at - 1, kTimeoutMs));
  TEST_ASSERT_TRUE(policy.restart_due(due_at, kTimeoutMs));
}

// A zero timeout is invalid and must never restart.
void test_zero_timeout_never_restarts(void) {
  WiFiWatchdogPolicy policy;
  policy.set_link_state(WiFiWatchdogLink::kUp, 1000);
  policy.set_link_state(WiFiWatchdogLink::kDown, 5000);
  TEST_ASSERT_FALSE(policy.restart_due(5000 + kTimeoutMs, 0));
}

// NetworkState mapping: only a routable station/Ethernet link is up. The
// soft-AP and the captive portal are neutral so they never arm the watchdog.
void test_link_mapping(void) {
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(WiFiWatchdogLink::kUp),
      static_cast<int>(
          wifi_watchdog_link_from_state(WiFiState::kWifiConnectedToAP)));
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(WiFiWatchdogLink::kDown),
      static_cast<int>(wifi_watchdog_link_from_state(WiFiState::kWifiNoAP)));
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(WiFiWatchdogLink::kDown),
      static_cast<int>(
          wifi_watchdog_link_from_state(WiFiState::kWifiDisconnected)));
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(WiFiWatchdogLink::kNeutral),
      static_cast<int>(
          wifi_watchdog_link_from_state(WiFiState::kWifiAPModeActivated)));
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(WiFiWatchdogLink::kNeutral),
      static_cast<int>(
          wifi_watchdog_link_from_state(WiFiState::kWifiManagerActivated)));
}

// AP+STA boot with an unreachable client network: soft-AP starts, station
// keeps failing. Must never restart.
void test_ap_sta_boot_without_station_never_restarts(void) {
  WiFiWatchdogPolicy policy;
  policy.set_link_state(
      wifi_watchdog_link_from_state(WiFiState::kWifiAPModeActivated), 500);
  for (uint32_t t = 1000; t < 20 * kTimeoutMs; t += 20000) {
    policy.set_link_state(
        wifi_watchdog_link_from_state(WiFiState::kWifiDisconnected), t);
    TEST_ASSERT_FALSE(policy.restart_due(t, kTimeoutMs));
  }
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_not_armed_never_restarts);
  RUN_TEST(test_armed_down_below_timeout_no_restart);
  RUN_TEST(test_armed_down_at_timeout_restarts);
  RUN_TEST(test_armed_up_never_restarts);
  RUN_TEST(test_reconnect_clears_timer);
  RUN_TEST(test_repeated_down_keeps_original_start);
  RUN_TEST(test_neutral_does_not_arm);
  RUN_TEST(test_neutral_clears_running_timer);
  RUN_TEST(test_clock_wraparound);
  RUN_TEST(test_zero_timeout_never_restarts);
  RUN_TEST(test_link_mapping);
  RUN_TEST(test_ap_sta_boot_without_station_never_restarts);
  return UNITY_END();
}
