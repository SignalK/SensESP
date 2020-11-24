#ifndef _led_light_H_
#define _led_light_H_

#include "system/configurable.h"
#include "system/valueconsumer.h"

/**
 * LEDLight is a special device object that can be used to control
 * an LED light.  It can control either a color or a monochrome
 * LED depending on how the pins are configured.  It can also be
 * configured with NO pins (-1), which can indicate there is no
 * actual LED connected to this device. This allows this class to
 * be used for optional LEDs and to function properly when one
 * is not present.
 * <p>Color LEDs can be controlled by either setting the specific
 * color to be displayed via the set_input(long), or by setting the
 * on/off state via set_input(bool). When specifying a simple on/off
 * via the the bool input, the default colors specified in the 
 * constructor are used.
 */
class LEDLight : public Configurable,
                 public ValueConsumer<long>,
                 public ValueConsumer<bool> {
                     
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
      * @param config_path The configuration path to use to load and save the
      *   user configurable values of this device. Use BLANK if this device
      *   can not have user configurable values.
      * @param led_on_rgb The RGB color to use when the switch is in the "on" state.
      *   For monochrome LEDs any non-zero value will light the LED. The default
      *   value is 0x00FF00, which is RGB for "green"
      * @param led_off_rgb The RGB color to use when the switch is in the "off" state.
      *   For monochrome LEDs any non-zero value will light the LED.  The default
      *   value is 0, RGB black, which is "off"       
      */    
      LEDLight(int led_r_pin = -1, int led_g_pin = -1, int led_b_pin = -1,
               String config_path = "",
               long led_on_rgb = 0x00FF00, long led_off_rgb = 0);


      /**
       * Used to set the "ON" color when turning the LED on or off via
       * the bool input of set_input(bool). This value overrides the
       * value specified in the constructor or user configuration.
       */
      void set_on_rgb(long new_rgb_value) { led_on_rgb = new_rgb_value; }


      /**
       * Used to set the "OFF" color when turning the LED on or off via
       * the bool input of set_input(bool). This value overrides the
       * value specified in the constructor or user configuration.
       */
      void set_off_rgb(long new_rgb_value) { led_off_rgb = new_rgb_value; }



      /**
       * Used to set the current display state of the LED. For a color
       * LED, new_value should be the RGB color to display. For a monocrhome
       * LED, new_value should be any non-zero color for "ON", and zero for
       * "OFF"
       */ 
      virtual void set_input(long new_value, uint8_t input_channel = 0) override;


      /**
       * Used to set the current display state of the LED with a simple on/off
       * boolean value.  Using TRUE for new_value sets the color to the ON color.
       * Using FALSE uses the OFF color.
       */ 
      virtual void set_input(bool new_value, uint8_t input_channel = 0) override;

      virtual void get_configuration(JsonObject& doc) override;
      virtual bool set_configuration(const JsonObject& config) override;
      virtual String get_config_schema() override;

    protected:
       int led_r_pin;
       int led_g_pin;
       int led_b_pin;
       long led_on_rgb;
       long led_off_rgb;
};

#endif