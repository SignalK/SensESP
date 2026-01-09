#ifndef SENSESP_SYSTEM_CRGB_H_
#define SENSESP_SYSTEM_CRGB_H_

#include <cstdint>

namespace sensesp {

/**
 * @brief Gamma correction lookup table (γ=2.2, inverted).
 *
 * Pre-computed values for inverse gamma correction to avoid expensive
 * floating-point math in the hot path. Generated with:
 *   table[i] = round(pow(i/255.0, 1/2.2) * 255)
 */
// clang-format off
constexpr uint8_t kGammaTable[256] = {
    0,  21,  28,  34,  39,  43,  46,  50,  53,  56,  59,  61,  64,  66,  68,  70,
   72,  74,  76,  78,  80,  82,  84,  85,  87,  89,  90,  92,  93,  95,  96,  98,
   99, 101, 102, 103, 105, 106, 107, 109, 110, 111, 112, 114, 115, 116, 117, 118,
  119, 120, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135,
  136, 137, 138, 139, 140, 141, 142, 143, 144, 144, 145, 146, 147, 148, 149, 150,
  150, 151, 152, 153, 154, 155, 155, 156, 157, 158, 159, 159, 160, 161, 162, 162,
  163, 164, 165, 165, 166, 167, 167, 168, 169, 170, 170, 171, 172, 172, 173, 174,
  174, 175, 176, 176, 177, 178, 178, 179, 180, 180, 181, 181, 182, 183, 183, 184,
  185, 185, 186, 186, 187, 188, 188, 189, 189, 190, 190, 191, 192, 192, 193, 193,
  194, 194, 195, 196, 196, 197, 197, 198, 198, 199, 199, 200, 200, 201, 201, 202,
  202, 203, 203, 204, 204, 205, 205, 206, 206, 207, 207, 208, 208, 209, 209, 210,
  210, 211, 211, 212, 212, 213, 213, 214, 214, 215, 215, 215, 216, 216, 217, 217,
  218, 218, 219, 219, 219, 220, 220, 221, 221, 222, 222, 222, 223, 223, 224, 224,
  225, 225, 225, 226, 226, 227, 227, 227, 228, 228, 229, 229, 229, 230, 230, 231,
  231, 231, 232, 232, 233, 233, 233, 234, 234, 234, 235, 235, 236, 236, 236, 237,
  237, 237, 238, 238, 238, 239, 239, 240, 240, 240, 241, 241, 241, 242, 242, 255,
};
// clang-format on

/**
 * @brief Apply gamma correction to an 8-bit value.
 *
 * LED brightness perception is non-linear - human eyes perceive brightness
 * logarithmically. This function applies inverse gamma correction to boost
 * low brightness values, making dim LEDs more visible.
 *
 * Uses a lookup table for efficiency (called at 200Hz).
 *
 * @param value Linear input value (0-255)
 * @return Gamma-corrected value (0-255)
 */
inline uint8_t gamma_correct(uint8_t value) {
  return kGammaTable[value];
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
 * @brief Apply gamma correction and brightness scaling to a color value.
 *
 * Gamma correction is applied first to ensure perceptually correct colors,
 * then brightness scaling is applied as a linear dimming factor.
 *
 * @param value Color value (0-255)
 * @param brightness Brightness scale (0-255), where 255 = full brightness
 * @return Scaled value (0-255)
 */
inline uint8_t apply_brightness(uint8_t value, uint8_t brightness) {
  return static_cast<uint8_t>(gamma_correct(value) * brightness / 255);
}

/**
 * @brief Convert RGB color to perceptual luminance.
 *
 * Uses the standard luminance formula (ITU-R BT.709) that weights
 * green most heavily, followed by red, then blue.
 *
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @return Luminance value (0-255)
 */
inline uint8_t rgb_to_luminance(uint8_t r, uint8_t g, uint8_t b) {
  return static_cast<uint8_t>(0.2126 * r + 0.7152 * g + 0.0722 * b);
}

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
