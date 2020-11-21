#include "smart_switch.h"


// Calculate the PWM value to send to analogWrite() based on the specified
// color value. When using analogWrite(), the closer to zero, the
// brighter the color. The closer to PWMRANGE, the darker the
// color.
static int getPWM(long rgb, int shift_right) {
    int color_val = (rgb >> shift_right) & 0xFF;
    float color_pct = color_val / 255.0;

    int colorRange = PWMRANGE / 2;

    return (int)(PWMRANGE - colorRange * color_pct);
}


SmartSwitch::SmartSwitch(int relay_pin, 
                   int led_r_pin, int led_g_pin, int led_b_pin,
                   long led_on_rgb, long led_off_rgb,
                   int enable_priority) :
    relay_pin{relay_pin},
    led_r_pin{led_r_pin},
    led_g_pin{led_g_pin},
    led_b_pin{led_b_pin} {

    this->set_priority(enable_priority);

    pinMode(relay_pin, OUTPUT);
    if (led_r_pin >= 0) {
       pinMode(led_r_pin, OUTPUT);
       if (led_g_pin >= 0) {
          // We are using a color RGB LED
          pinMode(led_g_pin, OUTPUT);
          pinMode(led_b_pin, OUTPUT);

          // Pre-compute PWM values for color components...
          pwm[0][0] = getPWM(led_off_rgb, 16);
          pwm[0][1] = getPWM(led_off_rgb, 8);
          pwm[0][2] = getPWM(led_off_rgb, 0);

          pwm[1][0] = getPWM(led_on_rgb, 16);
          pwm[1][1] = getPWM(led_on_rgb, 8);
          pwm[1][2] = getPWM(led_on_rgb, 0);
       }
       else {
          // Using a monochrome LED
          pwm[0][0] = (led_off_rgb != 0 ? HIGH : LOW);
          pwm[1][0] = (led_on_rgb != 0 ? HIGH : LOW);
       }
    }
}


void SmartSwitch::set_input(bool new_value, uint8_t input_channel) {

    if (new_value != this->output) {
        // The state has changed...
        set_state(new_value);
        notify();
    }

}


void SmartSwitch::enable() {
   set_state(false);
}


void SmartSwitch::set_state(bool new_value) {
    this->output = new_value;
    digitalWrite(relay_pin, this->output ? HIGH : LOW);
    if (led_r_pin >= 0) {
        // We are using a status LED
        int ledOn = this->output ? 1 : 0;
        if (led_g_pin >= 0) {
            // We are using an RGB LED
            analogWrite(led_r_pin, pwm[ledOn][0]);
            analogWrite(led_g_pin, pwm[ledOn][1]);
            analogWrite(led_b_pin, pwm[ledOn][2]);
        }
        else {
            // We are using a monochrome LED
            digitalWrite(led_r_pin, pwm[ledOn][0]);
        }
    }
}
