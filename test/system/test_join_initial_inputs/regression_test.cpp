#include <Arduino.h>
#include <unity.h>

#include "sensesp/transforms/join.h"

using namespace sensesp;

void test_join_waits_for_both_inputs() {
  Join<int, int> join;
  int notifications = 0;
  join.attach([&]() { ++notifications; });
  std::get<0>(join.consumers).set(7);
  TEST_ASSERT_EQUAL_INT(0, notifications);
  std::get<1>(join.consumers).set(9);
  TEST_ASSERT_EQUAL_INT(1, notifications);
  TEST_ASSERT_EQUAL_INT(7, std::get<0>(join.get()));
  TEST_ASSERT_EQUAL_INT(9, std::get<1>(join.get()));
  std::get<0>(join.consumers).set(8);
  TEST_ASSERT_EQUAL_INT(2, notifications);
}
void test_join5_waits_for_all_inputs() {
  Join5<int, int, int, int, int> join(1000);
  int notifications = 0;
  join.attach([&]() { ++notifications; });
  std::get<0>(join.consumers).set(1);
  std::get<1>(join.consumers).set(2);
  std::get<2>(join.consumers).set(3);
  std::get<3>(join.consumers).set(4);
  TEST_ASSERT_EQUAL_INT(0, notifications);
  std::get<4>(join.consumers).set(5);
  TEST_ASSERT_EQUAL_INT(1, notifications);
}
void test_join_rejects_an_expired_input() {
  Join<int, int> join(10);
  int notifications = 0;
  join.attach([&]() { ++notifications; });
  std::get<0>(join.consumers).set(1);
  delay(20);
  std::get<1>(join.consumers).set(2);
  TEST_ASSERT_EQUAL_INT(0, notifications);
  std::get<0>(join.consumers).set(3);
  TEST_ASSERT_EQUAL_INT(1, notifications);
}
void test_join3_and_join4_wait_for_all_inputs() {
  Join3<int, int, int> three;
  Join4<int, int, int, int> four;
  int three_notifications = 0, four_notifications = 0;
  three.attach([&]() { ++three_notifications; });
  four.attach([&]() { ++four_notifications; });
  std::get<0>(three.consumers).set(1);
  std::get<1>(three.consumers).set(2);
  TEST_ASSERT_EQUAL_INT(0, three_notifications);
  std::get<2>(three.consumers).set(3);
  TEST_ASSERT_EQUAL_INT(1, three_notifications);
  std::get<0>(four.consumers).set(1);
  std::get<1>(four.consumers).set(2);
  std::get<2>(four.consumers).set(3);
  TEST_ASSERT_EQUAL_INT(0, four_notifications);
  std::get<3>(four.consumers).set(4);
  TEST_ASSERT_EQUAL_INT(1, four_notifications);
}
void setUp() {}
void tearDown() {}
void setup() {
  delay(2000);
  UNITY_BEGIN();
  RUN_TEST(test_join_rejects_an_expired_input);
  RUN_TEST(test_join3_and_join4_wait_for_all_inputs);
  RUN_TEST(test_join_waits_for_both_inputs);
  RUN_TEST(test_join5_waits_for_all_inputs);
  UNITY_END();
}
void loop() {}
