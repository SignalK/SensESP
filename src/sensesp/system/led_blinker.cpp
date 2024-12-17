#include "led_blinker.h"

#include "FastLED.h"

namespace sensesp {

LEDPatternFragment frag_solid_color(uint32_t duration_ms, const CRGB& color) {
  return LEDPatternFragment(duration_ms,
                            [color](uint32_t, CRGB& crgb) { crgb = color; });
}

LEDPatternFragment frag_linear_fade(uint32_t duration_ms,
                                    uint32_t fade_duration_ms,
                                    const CRGB& target_color) {
  LEDPatternFragment fragment(duration_ms, [target_color, fade_duration_ms](
                                               uint32_t elapsed_ms,
                                               CRGB& crgb) {
    static CRGB from_color;
    static unsigned long last_elapsed_ms = 0;
    if (elapsed_ms < last_elapsed_ms) {
      from_color = crgb;
    }
    last_elapsed_ms = elapsed_ms;
    if (elapsed_ms >= fade_duration_ms) {
      crgb = target_color;
      return;
    }
    crgb = blend(from_color, target_color, elapsed_ms * 256 / fade_duration_ms);
  });

  return fragment;
}

LEDPatternFragment frag_linear_invert(uint32_t duration_ms, bool reverse) {
  return LEDPatternFragment(
      duration_ms, [duration_ms, reverse](uint32_t elapsed_ms, CRGB& crgb) {
        // Blend the color from current color to inverted
        if (reverse) {
          crgb = blend(CRGB(255, 255, 255) - crgb, crgb,
                       elapsed_ms * 256 / duration_ms);
        } else {
          crgb = blend(crgb, CRGB(255, 255, 255) - crgb,
                       elapsed_ms * 256 / duration_ms);
        }
      });
}

LEDPatternFragment frag_blend(uint32_t duration_ms, const CRGB& target_color,
                              bool reverse) {
  return LEDPatternFragment(duration_ms, [duration_ms, reverse, target_color](
                                             uint32_t elapsed_ms, CRGB& crgb) {
    if (reverse) {
      crgb = blend(target_color, crgb, elapsed_ms * 255 / duration_ms);
    } else {
      crgb = blend(crgb, target_color, elapsed_ms * 255 / duration_ms);
    }
  });
}

LEDPatternFragment frag_nop(uint32_t duration_ms) {
  // No operation
  return LEDPatternFragment(duration_ms, [](uint32_t, CRGB&) {});
}

}  // namespace sensesp
