#include "smart_switch.h"

SmartSwitch::SmartSwitch(int relay_pin, 
                   int led_r_pin, int led_g_pin, int led_b_pin,
                   long led_on_rgb, long led_off_rgb,
                   int enable_priority) :
    relay_pin{relay_pin},
    led_on_rgb{led_on_rgb},
    led_off_rgb{led_off_rgb} {

    this->set_priority(enable_priority);

    pinMode(relay_pin, OUTPUT);
    led_light = new LEDLight(led_r_pin, led_g_pin, led_b_pin);

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
    led_light->set_input(new_value ? led_on_rgb : led_off_rgb);
}
