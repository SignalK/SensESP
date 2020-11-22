#ifndef _led_color_H_
#define _led_color_H_

#include "system/valueconsumer.h"

/**
 * LEDLight is a special device object that can be used to control
 * an LED light.  It can control either a color or a monochrome
 * LED depending on how the pins are configured.  It can also be
 * configured with NO pins (-1), which can indicate there is no
 * actual LED connected to this device. This allows this class to
 * be used for optional LEDs and to function properly when one
 * is not present.
 */
class LEDLight : public ValueConsumer<long> {
                     
    public:
     /**
      * The constructor
      * @param led_r_pin the digital output pin that controls the red element of
      *   an RGB LED OR the only pin used for a status LED for a monochrome LED.
      *   -1 can be used to indicate there is no LED attached and values should be ignored
      * @param led_g_pin the digital output pin that controls the green element of
      *   an RGB LED.  Set to -1 if a monochrome LED is being used.
      * @param led_b_pin the digital output pin that controls the blue element of
      *   an RGB LED.  Set to -1 if a monochrome LED is being used.
      */    
      LEDLight(int led_r_pin = -1, int led_g_pin = -1, int led_b_pin = -1);


      /**
       * Used to set the current display state of the LED. For a color
       * LED, new_value should be the RGB color to display. For a monocrhome
       * LED, new_value should be any non-zero color for "ON", and zero for
       * "OFF"
       */ 
      virtual void set_input(long new_value, uint8_t input_channel = 0) override;

     protected:
       int led_r_pin;
       int led_g_pin;
       int led_b_pin;
};

#endif