#include "rgb_led.h"

#include "pwm_output.h"

namespace sensesp {

constexpr int kPWMResolution = 13;

RgbLed::RgbLed(int led_r_pin, int led_g_pin, int led_b_pin, String config_path,
               long int led_on_rgb, long int led_off_rgb, bool common_anode)
    : FileSystemSaveable(config_path),
      led_on_rgb_{led_on_rgb},
      led_off_rgb_{led_off_rgb},
      common_anode_{common_anode} {
  this->load();

  ledcAttach(led_r_pin, 5000, kPWMResolution);
  ledcAttach(led_g_pin, 5000, kPWMResolution);
  ledcAttach(led_b_pin, 5000, kPWMResolution);
}

// Calculate the PWM value to send to analogWrite() based on the specified
// color value. When using analogWrite(), the closer to zero, the
// brighter the color. The closer to PWMRANGE, the darker the
// color.
static float get_pwm(int64_t rgb, int shift_right, bool common_anode) {
  int color_val = (rgb >> shift_right) & 0xFF;
  float const color_pct = color_val / 255.0;

  if (common_anode) {
    return 1.0 - color_pct;
  }
  return color_pct;
}

bool RgbLed::to_json(JsonObject& root) {
  root["led_on_rgb"] = led_on_rgb_;
  root["led_off_rgb"] = led_off_rgb_;
  return true;
}

bool RgbLed::from_json(const JsonObject& config) {
  String expected[] = {"led_on_rgb", "led_off_rgb"};
  for (auto str : expected) {
    if (!config[str].is<JsonVariant>()) {
      return false;
    }
  }
  led_on_rgb_ = config["led_on_rgb"];
  led_off_rgb_ = config["led_off_rgb"];
  return true;
}

void RgbLed::set_color(long new_value) {
  float r = get_pwm(new_value, 16, common_anode_);
  const int max_brightness = 1 << kPWMResolution;
  ledcWrite(led_r_pin_, static_cast<int>(r * max_brightness));

  float g = get_pwm(new_value, 8, common_anode_);
  ledcWrite(led_g_pin_, static_cast<int>(g * max_brightness));

  float b = get_pwm(new_value, 0, common_anode_);
  ledcWrite(led_b_pin_, static_cast<int>(b * max_brightness));
}

const String ConfigSchema(const RgbLed& obj) {
  return R"({"type":"object","properties":{"led_on_rgb":{"title":"RGB color for led ON","type":"integer"},"led_off_rgb":{"title":"RGB color for led OFF","type":"integer"}}})";
}

}  // namespace sensesp
