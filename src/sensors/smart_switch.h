#ifndef _smart_switch_H_
#define _smart_switch_H_

#include "transforms/transform.h"

class SmartSwitchController;

/**
 * SmartSwitch is a specialized digital output controller intended to be wired to both
 * a relay that controls a load, as well as an optional LED to display its current status.
 * It consumes boolean values used to set the state of the switch. After setting the status
 * of the relay (and optional LED), the value is the forwarded along, as this device
 * is also a BooleanProducer.  Note that values will only be produced if an actual
 * change occurs on the SmartSwitch.
 * <p>The attached LED may be either a single color LED (attached to a single output pin),
 * or a multi-color RGB LED. To specify a monochrome LED, pass a single pin number for
 * led_r_pin, and leave the others undefined as -1.
 * @see SmartSwitchController
 */
class SmartSwitch : public BooleanTransform {

   public:
       /**
        * The constructor
        * @param relay_pin the digital output pin that controls the relay
        * @param led_r_pin the digital output pin that controls the red element of
        *   an RGB LED OR the only pin used for a status LED for a monochrome LED.
        * @param led_g_pin the digital output pin that controls the green element of
        *   an RGB LED.  Set to -1 if a monochrome LED is being used.
        * @param led_b_pin the digital output pin that controls the blue element of
        *   an RGB LED.  Set to -1 if a monochrome LED is being used.
        * @param led_on_rgb The RGB color to use when the switch is in the "on" state.
        *   For monochrome LEDs any non-zero value will light the LED.
        * @param led_off_rgb The RGB color to use when the switch is in the "off" state.
        *   For monochrome LEDs any non-zero value will light the LED.
        * @param enable_priority The "enable priority" used when this switch is enabled. 
        * @see Enable
        */
       SmartSwitch(int relay_pin, 
                   int led_r_pin = -1, int led_g_pin = -1, int led_b_pin = -1,
                   long led_on_rgb = 0x00FF00, long led_off_rgb = 0,
                   int enable_priority=1);

       void set_input(bool new_value, uint8_t input_channel = 0) override;
       void enable() override;


       /**
        * set_state() allows the state of the switch to be set without any
        * type of broadcast to observers. This is usually used for initialization
        * as well as resetting.
        */
       void set_state(bool new_value);

   protected:
       int relay_pin;
       int led_r_pin;
       int led_g_pin;
       int led_b_pin;
       int pwm[2][3];
};

#endif