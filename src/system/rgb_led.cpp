#include "rgb_led.h"

#ifdef ESP32
#include "system/analog_write_esp32.h"
#endif

RgbLed::RgbLed(int led_r_pin, int led_g_pin, int led_b_pin, String config_path,
               long led_on_rgb, long led_off_rgb)
    : Configurable(config_path),
      led_r_pin{led_r_pin},
      led_g_pin{led_g_pin},
      led_b_pin{led_b_pin},
      led_on_rgb{led_on_rgb},
      led_off_rgb{led_off_rgb} {
  pinMode(led_r_pin, OUTPUT);
  pinMode(led_g_pin, OUTPUT);
  pinMode(led_b_pin, OUTPUT);

  this->load_configuration();
}

// Calculate the PWM value to send to analogWrite() based on the specified
// color value. When using analogWrite(), the closer to zero, the
// brighter the color. The closer to PWMRANGE, the darker the
// color.
static int get_pwm(long rgb, int shift_right) {
  int color_val = (rgb >> shift_right) & 0xFF;
  float color_pct = color_val / 255.0;

  int colorRange = PWMRANGE / 2;

  return (int)(PWMRANGE - colorRange * color_pct);
}

void RgbLed::set_input(long new_value, uint8_t input_channel) {
  int r = get_pwm(new_value, 16);
  int g = get_pwm(new_value, 8);
  int b = get_pwm(new_value, 0);
  analogWrite(led_r_pin, r);
  analogWrite(led_g_pin, g);
  analogWrite(led_b_pin, b);
}

void RgbLed::set_input(bool new_value, uint8_t input_channel) {
  if (new_value) {
    set_input(led_on_rgb, input_channel);
  } else {
    set_input(led_off_rgb, input_channel);
  }
}

void RgbLed::get_configuration(JsonObject& root) {
  root["led_on_rgb"] = led_on_rgb;
  root["led_off_rgb"] = led_off_rgb;
}

static const char SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "led_on_rgb": { "title": "RGB color for led ON", "type": "integer" },
        "led_off_rgb": { "title": "RGB color for led OFF", "type": "integer" }
    }
  })";

String RgbLed::get_config_schema() { return FPSTR(SCHEMA); }

bool RgbLed::set_configuration(const JsonObject& config) {
  String expected[] = {"led_on_rgb", "led_off_rgb"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  led_on_rgb = config["led_on_rgb"];
  led_off_rgb = config["led_off_rgb"];
  return true;
}