#include "led_light.h"


LEDLight::LEDLight(int led_r_pin, int led_g_pin, int led_b_pin,
                   String config_path,
                   long led_on_rgb, long led_off_rgb) :
    Configurable(config_path),
    led_r_pin{led_r_pin},
    led_g_pin{led_g_pin},
    led_b_pin{led_b_pin},
    led_on_rgb{led_on_rgb},
    led_off_rgb{led_off_rgb} {

    if (led_r_pin >= 0) {
       pinMode(led_r_pin, OUTPUT);
       if (led_g_pin >= 0) {
          // We are using a color RGB LED
          pinMode(led_g_pin, OUTPUT);
          pinMode(led_b_pin, OUTPUT);
       }
    }

    this->load_configuration();
}


#ifdef ESP32

// Code for analogWrite() simulation on ESP32 inspired by
// https://github.com/ERROPiX/ESP32_AnalogWrite

int8_t channel_to_pin[16] = { -1, -1, -1, -1,
                              -1, -1, -1, -1,
                              -1, -1, -1, -1,
                              -1, -1, -1, -1
};

#define CHANNEL_FREQUENCY 5000
#define CHANNEL_RESOLUTION 13

static int pin_to_channel(uint8_t pin)
{
  int channel = -1;

  // Check if pin already attached to a channel
  for (uint8_t i = 0; i < 16; i++)
  {
    if (channel_to_pin[i] == pin)
    {
      channel = i;
      break;
    }
  }

  // If not, attach it to a free channel
  if (channel == -1)
  {
    for (uint8_t i = 0; i < 16; i++)
    {
      if (channel_to_pin[i] == -1)
      {
        channel_to_pin[i] = pin;
        channel = i;
        ledcSetup(channel, CHANNEL_FREQUENCY, CHANNEL_RESOLUTION);
        ledcAttachPin(pin, channel);
        break;
      }
    }
  }

  return channel;
}

static void analogWrite(uint8_t pin, uint32_t value, uint32_t valueMax = 255)
{
  int channel = pin_to_channel(pin);

  // Make sure the pin was attached to a channel, if not do nothing
  if (channel != -1 && channel < 16)
  {
    uint32_t levels = pow(2, CHANNEL_RESOLUTION);
    uint32_t duty = ((levels - 1) / valueMax) * min(value, valueMax);

    // write duty to LEDC
    ledcWrite(channel, duty);
  }
}


// Calculate the PWM value to send to analogWrite() based on the specified
// color value. 
static int get_pwm(long rgb, int shift_right) {
    int color_val = (rgb >> shift_right) & 0xFF;
    return 255 - color_val;
}

#else

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

#endif

void LEDLight::set_input(long new_value, uint8_t input_channel) {

    if (led_r_pin >= 0) {
        // An LED is connected
        if (led_g_pin >= 0) {
            // And its color
            int r = get_pwm(new_value, 16);
            int g = get_pwm(new_value, 8);
            int b = get_pwm(new_value, 0);
            analogWrite(led_r_pin, r);
            analogWrite(led_g_pin, g);
            analogWrite(led_b_pin, b);
        }
        else {
            // The LED is a simple monochrome ON/OFF
            int state = (new_value != 0 ? HIGH : LOW);
            digitalWrite(led_r_pin, state);
        }
    }
}


void LEDLight::set_input(bool new_value, uint8_t input_channel) {

   if (new_value) {
      set_input(led_on_rgb, input_channel);
   }
   else {
      set_input(led_off_rgb, input_channel);
   }

}


void LEDLight::get_configuration(JsonObject& root) {
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


String LEDLight::get_config_schema() { return FPSTR(SCHEMA); }

bool LEDLight::set_configuration(const JsonObject& config) {
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