/**
 * @file ws_connect_due_test.cpp
 * @brief Host unit tests for the reconnect-attempt timing check
 *        (signalk_ws_connect_due.h).
 *
 * Runs on the `native` env (no Arduino):
 *   pio test -e native -f native/test_ws_connect_due
 *
 * millis() is 32 bits and wraps after about 49.7 days. The time of the last
 * attempt is kept for as long as the connection lasts, which on a boat can be
 * weeks, so the check must stay correct after long connections and across a
 * millis() wrap.
 */

#include <unity.h>

#include <cstdint>

#include "sensesp/signalk/signalk_ws_connect_due.h"

using namespace sensesp;

constexpr uint32_t kWaitMs = 2000;
constexpr uint32_t kDayMs = 24UL * 60 * 60 * 1000;
constexpr uint32_t kNearWrap = UINT32_MAX - 1000;

void test_before_wait_is_not_due(void) {
  TEST_ASSERT_FALSE(sk_connect_due(1000 + kWaitMs - 1, 1000, kWaitMs));
}

void test_at_wait_is_due(void) {
  TEST_ASSERT_TRUE(sk_connect_due(1000 + kWaitMs, 1000, kWaitMs));
}

// A wait that spans the millis() wrap ends on time, not early or late.
void test_wait_across_millis_wrap(void) {
  uint32_t last_attempt = kNearWrap;
  TEST_ASSERT_FALSE(sk_connect_due(last_attempt + kWaitMs - 1, last_attempt,
                                   kWaitMs));
  TEST_ASSERT_TRUE(sk_connect_due(last_attempt + kWaitMs, last_attempt,
                                  kWaitMs));
}

// The last attempt was late in the first millis() cycle and the connection
// outlived the wrap: a disconnect now must be retried right away.
void test_due_after_millis_wrap(void) {
  TEST_ASSERT_TRUE(sk_connect_due(10000, kNearWrap, kWaitMs));
}

// A connection that lasted 30 days, longer than half the millis() range, must
// not hold off the retry.
void test_due_after_30_day_connection(void) {
  TEST_ASSERT_TRUE(sk_connect_due(1000 + 30 * kDayMs, 1000, kWaitMs));
}

void setUp(void) {}
void tearDown(void) {}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_before_wait_is_not_due);
  RUN_TEST(test_at_wait_is_due);
  RUN_TEST(test_wait_across_millis_wrap);
  RUN_TEST(test_due_after_millis_wrap);
  RUN_TEST(test_due_after_30_day_connection);
  return UNITY_END();
}
