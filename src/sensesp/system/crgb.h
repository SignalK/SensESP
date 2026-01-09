#ifndef SENSESP_SYSTEM_CRGB_H_
#define SENSESP_SYSTEM_CRGB_H_

#include <cstdint>
#include <cmath>

namespace sensesp {

/**
 * @brief Apply gamma correction to an 8-bit value.
 *
 * LED brightness perception is non-linear - human eyes perceive brightness
 * logarithmically. This function applies inverse gamma correction to boost
 * low brightness values, making dim LEDs more visible.
 *
 * @param value Linear input value (0-255)
 * @param gamma Gamma exponent (typical: 2.2-2.8 for LEDs, inverted internally)
 * @return Gamma-corrected value (0-255)
 */
inline uint8_t gamma_correct(uint8_t value, float gamma = 2.8f) {
  // Apply inverse gamma to boost low values
  return static_cast<uint8_t>(std::pow(value / 255.0f, 1.0f / gamma) * 255.0f + 0.5f);
}

/**
 * @brief Minimal RGB color struct.
 *
 * This is a lightweight replacement for FastLED's CRGB struct,
 * implementing only the features used by SensESP's LED blinker system.
 */
struct CRGB {
  uint8_t r;
  uint8_t g;
  uint8_t b;

  /// Default constructor - initializes to black
  constexpr CRGB() noexcept : r(0), g(0), b(0) {}

  /// RGB constructor
  constexpr CRGB(uint8_t ir, uint8_t ig, uint8_t ib) noexcept
      : r(ir), g(ig), b(ib) {}

  /// Copy constructor
  constexpr CRGB(const CRGB& rhs) noexcept = default;

  /// Assignment operator
  CRGB& operator=(const CRGB& rhs) noexcept = default;

  /**
   * @brief Binary subtraction operator with saturation.
   *
   * Subtracts each channel component-wise, saturating at 0.
   * Used for color inversion: CRGB(255,255,255) - color
   */
  constexpr CRGB operator-(const CRGB& rhs) const noexcept {
    return CRGB(r > rhs.r ? r - rhs.r : 0, g > rhs.g ? g - rhs.g : 0,
                b > rhs.b ? b - rhs.b : 0);
  }

  // Predefined color constants
  static const CRGB Black;
  static const CRGB Red;
  static const CRGB Yellow;
  static const CRGB Blue;
};

// Static color constant definitions
inline constexpr CRGB CRGB::Black{0, 0, 0};
inline constexpr CRGB CRGB::Red{255, 0, 0};
inline constexpr CRGB CRGB::Yellow{255, 255, 0};
inline constexpr CRGB CRGB::Blue{0, 0, 255};

/**
 * @brief Linear interpolation (blend) between two colors.
 *
 * @param p1 First color (returned when amountOfP2 == 0)
 * @param p2 Second color (returned when amountOfP2 == 255)
 * @param amountOfP2 Blend amount: 0 = 100% p1, 255 = 100% p2
 * @return Blended color
 */
inline CRGB blend(const CRGB& p1, const CRGB& p2, uint8_t amountOfP2) {
  if (amountOfP2 == 0) {
    return p1;
  }
  if (amountOfP2 == 255) {
    return p2;
  }

  // Linear interpolation for each channel using integer math
  auto lerp8 = [](uint8_t a, uint8_t b, uint8_t frac) -> uint8_t {
    if (b > a) {
      uint8_t delta = b - a;
      uint8_t scaled = (static_cast<uint16_t>(delta) * frac) >> 8;
      return a + scaled;
    } else {
      uint8_t delta = a - b;
      uint8_t scaled = (static_cast<uint16_t>(delta) * frac) >> 8;
      return a - scaled;
    }
  };

  return CRGB(lerp8(p1.r, p2.r, amountOfP2), lerp8(p1.g, p2.g, amountOfP2),
              lerp8(p1.b, p2.b, amountOfP2));
}

}  // namespace sensesp

#endif  // SENSESP_SYSTEM_CRGB_H_
