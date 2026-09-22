/**
 * @file delta_packing_test.cpp
 * @brief Host unit tests for the delta packing accumulator
 *        (signalk_delta_packing.h).
 *
 * Runs on the `native` env (no Arduino / ArduinoJson):
 *   pio test -e native -f native/test_delta_packing
 *
 * The packer decides where one delta ends and the next begins, so that
 * get_deltas() splits a large batch instead of handing send_delta() a delta it
 * has to drop. The budget boundary matches sk_delta_exceeds_ws_buffer(): a
 * delta of exactly buffer_size is sent, so exactly budget fits.
 */

#include <unity.h>

#include "sensesp/signalk/signalk_delta_packing.h"

using namespace sensesp;

// The envelope is there before any item is added.
void test_empty_packer_holds_the_envelope(void) {
  SKDeltaPacker packer(40, 1024);
  TEST_ASSERT_EQUAL_UINT(0, packer.item_count());
  TEST_ASSERT_EQUAL_UINT(40, packer.length());
}

// Envelope plus the item, with no separator before the first one.
void test_first_item_adds_no_separator(void) {
  SKDeltaPacker packer(40, 1024);
  packer.add(60);
  TEST_ASSERT_EQUAL_UINT(1, packer.item_count());
  TEST_ASSERT_EQUAL_UINT(100, packer.length());
}

// Every later item pays for the comma that joins it to the previous one.
void test_later_items_pay_for_the_separator(void) {
  SKDeltaPacker packer(40, 1024);
  packer.add(60);
  packer.add(60);
  TEST_ASSERT_EQUAL_UINT(161, packer.length());
}

// A delta of exactly the budget is sendable, so it fits.
void test_exactly_the_budget_fits(void) {
  SKDeltaPacker packer(40, 100);
  TEST_ASSERT_TRUE(packer.fits(60));
  packer.add(60);
  TEST_ASSERT_EQUAL_UINT(100, packer.length());
}

// One byte over the budget belongs in the next delta.
void test_one_byte_over_the_budget_does_not_fit(void) {
  SKDeltaPacker packer(40, 100);
  TEST_ASSERT_FALSE(packer.fits(61));
}

// The separator is counted when deciding, not only when adding: two 30 B items
// fit in 101 B only if the comma is ignored.
void test_separator_counts_against_the_budget(void) {
  SKDeltaPacker packer(40, 100);
  packer.add(30);
  TEST_ASSERT_FALSE(packer.fits(30));
  SKDeltaPacker roomier(40, 101);
  roomier.add(30);
  TEST_ASSERT_TRUE(roomier.fits(30));
}

// An item longer than the budget fits nowhere. The packer says so rather than
// looping: the caller emits it alone and send_delta() drops it.
void test_item_larger_than_the_budget_never_fits(void) {
  SKDeltaPacker packer(40, 100);
  TEST_ASSERT_FALSE(packer.fits(61));
  TEST_ASSERT_TRUE(packer.is_empty());
}

// Budget 0 means no limit: everything fits.
void test_zero_budget_means_unlimited(void) {
  SKDeltaPacker packer(40, 0);
  packer.add(100000);
  TEST_ASSERT_TRUE(packer.fits(100000));
}

// reset() starts the next delta from the envelope again.
void test_reset_starts_the_next_delta(void) {
  SKDeltaPacker packer(40, 1024);
  packer.add(60);
  packer.reset();
  TEST_ASSERT_EQUAL_UINT(0, packer.item_count());
  TEST_ASSERT_EQUAL_UINT(40, packer.length());
}

// The whole point, end to end: five 60 B items under a 200 B budget split into
// three deltas of 2, 2 and 1, and every delta stays within the budget.
void test_batch_splits_into_deltas_within_the_budget(void) {
  const size_t kItems[] = {60, 60, 60, 60, 60};
  SKDeltaPacker packer(40, 200);
  size_t deltas = 0;
  size_t items_in_delta[8] = {0};
  for (size_t i = 0; i < 5; i++) {
    if (!packer.is_empty() && !packer.fits(kItems[i])) {
      TEST_ASSERT_TRUE(packer.length() <= 200);
      deltas++;
      packer.reset();
    }
    packer.add(kItems[i]);
    items_in_delta[deltas]++;
  }
  TEST_ASSERT_TRUE(packer.length() <= 200);
  deltas++;

  TEST_ASSERT_EQUAL_UINT(3, deltas);
  TEST_ASSERT_EQUAL_UINT(2, items_in_delta[0]);
  TEST_ASSERT_EQUAL_UINT(2, items_in_delta[1]);
  TEST_ASSERT_EQUAL_UINT(1, items_in_delta[2]);
}

void setUp(void) {}
void tearDown(void) {}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_empty_packer_holds_the_envelope);
  RUN_TEST(test_first_item_adds_no_separator);
  RUN_TEST(test_later_items_pay_for_the_separator);
  RUN_TEST(test_exactly_the_budget_fits);
  RUN_TEST(test_one_byte_over_the_budget_does_not_fit);
  RUN_TEST(test_separator_counts_against_the_budget);
  RUN_TEST(test_item_larger_than_the_budget_never_fits);
  RUN_TEST(test_zero_budget_means_unlimited);
  RUN_TEST(test_reset_starts_the_next_delta);
  RUN_TEST(test_batch_splits_into_deltas_within_the_budget);
  return UNITY_END();
}
