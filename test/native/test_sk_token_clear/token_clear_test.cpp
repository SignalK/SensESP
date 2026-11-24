#include "sensesp/signalk/signalk_ws_token_policy.h"
#include "unity.h"

using namespace sensesp;

// A 401 on the upgrade means the server rejected the token: discard it so the
// next reconnect re-requests access. The decision does not depend on the
// transport -- a plaintext deployment would otherwise have no recovery route
// and would loop forever on a stale token.
void test_clears_on_401() {
  TEST_ASSERT_TRUE(should_clear_token_on_status(401));
}

// Any status other than 401 keeps the token. 0 is the "not applicable" value
// reported by websocket clients that do not expose the handshake status.
void test_keeps_token_on_non_401() {
  TEST_ASSERT_FALSE(should_clear_token_on_status(0));
  TEST_ASSERT_FALSE(should_clear_token_on_status(200));
  TEST_ASSERT_FALSE(should_clear_token_on_status(426));
  TEST_ASSERT_FALSE(should_clear_token_on_status(500));
}

void setUp() {}
void tearDown() {}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_clears_on_401);
  RUN_TEST(test_keeps_token_on_non_401);
  return UNITY_END();
}

