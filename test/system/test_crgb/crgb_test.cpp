/**
 * @file crgb_test.cpp
 * @brief Unit tests for CRGB color functions (gamma correction, brightness, luminance)
 */

#include "sensesp/system/crgb.h"
#include "unity.h"

using namespace sensesp;

// Test gamma_correct() function
void test_gamma_correct_zero() {
  // Zero should remain zero
  TEST_ASSERT_EQUAL(0, gamma_correct(0));
}

void test_gamma_correct_max() {
  // 255 should remain 255
  TEST_ASSERT_EQUAL(255, gamma_correct(255));
}

void test_gamma_correct_boosts_low_values() {
  // Inverse gamma should boost low values
  // For example, 40 should become ~110 (from the lookup table)
  uint8_t result = gamma_correct(40);
  TEST_ASSERT_GREATER_THAN(40, result);
  TEST_ASSERT_EQUAL(110, result);  // Exact value from table
}

void test_gamma_correct_monotonic() {
  // Gamma correction should be monotonically increasing
  for (int i = 1; i < 256; i++) {
    TEST_ASSERT_GREATER_OR_EQUAL(gamma_correct(i - 1), gamma_correct(i));
  }
}

// Test apply_brightness() function
void test_apply_brightness_full() {
  // Full brightness (255) should give gamma-corrected value
  TEST_ASSERT_EQUAL(gamma_correct(100), apply_brightness(100, 255));
  TEST_ASSERT_EQUAL(gamma_correct(255), apply_brightness(255, 255));
}

void test_apply_brightness_zero() {
  // Zero brightness should always give 0
  TEST_ASSERT_EQUAL(0, apply_brightness(0, 0));
  TEST_ASSERT_EQUAL(0, apply_brightness(255, 0));
  TEST_ASSERT_EQUAL(0, apply_brightness(128, 0));
}

void test_apply_brightness_scaling() {
  // With brightness=40 and color=255:
  // gamma_correct(255) = 255, then 255 * 40 / 255 = 40
  uint8_t result = apply_brightness(255, 40);
  TEST_ASSERT_EQUAL(40, result);
}

void test_apply_brightness_order() {
  // This test verifies the fix: gamma should be applied BEFORE brightness
  // Old (wrong): gamma_correct(40 * 255 / 255) = gamma_correct(40) = 110
  // New (correct): gamma_correct(255) * 40 / 255 = 255 * 40 / 255 = 40
  uint8_t result = apply_brightness(255, 40);
  TEST_ASSERT_EQUAL(40, result);  // Should be 40, not 110
}

// Test rgb_to_luminance() function
void test_rgb_to_luminance_black() {
  TEST_ASSERT_EQUAL(0, rgb_to_luminance(0, 0, 0));
}

void test_rgb_to_luminance_white() {
  // 0.2126*255 + 0.7152*255 + 0.0722*255 = 255
  TEST_ASSERT_EQUAL(255, rgb_to_luminance(255, 255, 255));
}

void test_rgb_to_luminance_red() {
  // Pure red: 0.2126 * 255 = 54.2 ≈ 54
  uint8_t result = rgb_to_luminance(255, 0, 0);
  TEST_ASSERT_INT_WITHIN(1, 54, result);
}

void test_rgb_to_luminance_green() {
  // Pure green: 0.7152 * 255 = 182.4 ≈ 182
  uint8_t result = rgb_to_luminance(0, 255, 0);
  TEST_ASSERT_INT_WITHIN(1, 182, result);
}

void test_rgb_to_luminance_blue() {
  // Pure blue: 0.0722 * 255 = 18.4 ≈ 18
  uint8_t result = rgb_to_luminance(0, 0, 255);
  TEST_ASSERT_INT_WITHIN(1, 18, result);
}

void test_rgb_to_luminance_green_dominant() {
  // Green should contribute most to luminance
  uint8_t red_lum = rgb_to_luminance(100, 0, 0);
  uint8_t green_lum = rgb_to_luminance(0, 100, 0);
  uint8_t blue_lum = rgb_to_luminance(0, 0, 100);

  TEST_ASSERT_GREATER_THAN(red_lum, green_lum);
  TEST_ASSERT_GREATER_THAN(blue_lum, green_lum);
  TEST_ASSERT_GREATER_THAN(blue_lum, red_lum);
}

void setup() {
  UNITY_BEGIN();

  // Gamma correction tests
  RUN_TEST(test_gamma_correct_zero);
  RUN_TEST(test_gamma_correct_max);
  RUN_TEST(test_gamma_correct_boosts_low_values);
  RUN_TEST(test_gamma_correct_monotonic);

  // Brightness application tests
  RUN_TEST(test_apply_brightness_full);
  RUN_TEST(test_apply_brightness_zero);
  RUN_TEST(test_apply_brightness_scaling);
  RUN_TEST(test_apply_brightness_order);

  // Luminance calculation tests
  RUN_TEST(test_rgb_to_luminance_black);
  RUN_TEST(test_rgb_to_luminance_white);
  RUN_TEST(test_rgb_to_luminance_red);
  RUN_TEST(test_rgb_to_luminance_green);
  RUN_TEST(test_rgb_to_luminance_blue);
  RUN_TEST(test_rgb_to_luminance_green_dominant);

  UNITY_END();
}

void loop() {}
