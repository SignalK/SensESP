/**
 * @file led_blinker_test.cpp
 * @brief Tests for LEDPattern assignment and fragment index management.
 *
 * Reproduces the crash from issue #857: switching from a pattern with many
 * fragments to one with fewer causes an out-of-bounds access because
 * operator= didn't reset the fragment index.
 */

#include <Arduino.h>

#include "sensesp/system/led_blinker.h"
#include "unity.h"

using namespace sensesp;

// Helper: create a pattern with N solid-color fragments.
static LEDPattern make_pattern(int n, const CRGB& color = CRGB::Red) {
  std::vector<LEDPatternFragment> frags;
  for (int i = 0; i < n; i++) {
    frags.push_back(frag_solid_color(100, color));
  }
  return LEDPattern(frags);
}

// ---------------------------------------------------------------------------
// Switching from large pattern to small must not crash
// ---------------------------------------------------------------------------

void test_pattern_switch_large_to_small() {
  LEDPattern large = make_pattern(8);
  LEDPattern small = make_pattern(2);

  // Advance the large pattern's index past the small pattern's size.
  // Each fragment is 100ms, so after 500ms we should be at index 5.
  CRGB crgb = CRGB::Black;
  unsigned long start = millis();
  while (millis() - start < 500) {
    large.apply(crgb);
    delay(1);
  }

  // Assign small pattern over large — this previously left the stale index.
  large = small;

  // This must not crash (was reading freed memory at fragments_[5]).
  large.apply(crgb);
  TEST_ASSERT_TRUE(true);  // If we get here, no crash.
}

// ---------------------------------------------------------------------------
// Assignment resets fragment index to 0
// ---------------------------------------------------------------------------

void test_pattern_assignment_resets_index() {
  // Create two patterns with different colors to distinguish them.
  CRGB green{0, 255, 0};
  LEDPattern pattern_a = {frag_solid_color(100, CRGB::Red),
                          frag_solid_color(100, green)};
  LEDPattern pattern_b = {frag_solid_color(100, CRGB::Blue)};

  CRGB crgb = CRGB::Black;

  // Advance pattern_a past the first fragment.
  unsigned long start = millis();
  while (millis() - start < 150) {
    pattern_a.apply(crgb);
    delay(1);
  }
  // crgb should be green (second fragment).
  TEST_ASSERT_EQUAL(0, crgb.r);
  TEST_ASSERT_GREATER_THAN(0, crgb.g);

  // Assign pattern_b — should start from its first (and only) fragment.
  pattern_a = pattern_b;
  pattern_a.apply(crgb);
  TEST_ASSERT_GREATER_THAN(0, crgb.b);
}

// ---------------------------------------------------------------------------
// Basic apply works with solid color
// ---------------------------------------------------------------------------

void test_pattern_apply_solid_color() {
  LEDPattern pattern = {frag_solid_color(1000, CRGB::Red)};
  CRGB crgb = CRGB::Black;
  pattern.apply(crgb);
  TEST_ASSERT_EQUAL(255, crgb.r);
  TEST_ASSERT_EQUAL(0, crgb.g);
  TEST_ASSERT_EQUAL(0, crgb.b);
}

// ---------------------------------------------------------------------------
// LEDBlinker set_pattern doesn't crash when switching patterns
// ---------------------------------------------------------------------------

void test_blinker_set_pattern_no_crash() {
  CRGB crgb = CRGB::Black;
  bool show_called = false;
  LEDBlinker blinker(crgb, make_pattern(8),
                     [&show_called]() { show_called = true; });

  // Tick a few times to advance the index.
  for (int i = 0; i < 100; i++) {
    blinker.tick();
    delay(5);
  }

  // Switch to smaller pattern.
  blinker.set_pattern(make_pattern(2));

  // This must not crash.
  blinker.tick();
  TEST_ASSERT_TRUE(show_called);
}

// ---------------------------------------------------------------------------
// Test runner
// ---------------------------------------------------------------------------

void setup() {
  delay(2000);

  UNITY_BEGIN();

  RUN_TEST(test_pattern_apply_solid_color);
  RUN_TEST(test_pattern_switch_large_to_small);
  RUN_TEST(test_pattern_assignment_resets_index);
  RUN_TEST(test_blinker_set_pattern_no_crash);

  UNITY_END();
}

void loop() {}
