#ifndef _rgb_led_H_
#define _rgb_led_H_

#include "system/configurable.h"
#include "system/valueconsumer.h"

/**
 * RgbLed is a special device object that can be used to control
 * a color rgb LED light via 3 digital output channels
 * <p>Color LEDs can be controlled by either setting the specific
 * color to be displayed via the set_input(long), or by setting the
 * on/off state via set_input(bool). When specifying a simple on/off
 * via the the bool input, the default ON or OFF colors specified in the
 * constructor are used.
 */
class RgbLed : public Configurable,
               public ValueConsumer<long>,
               public ValueConsumer<bool> {
 public:
  /**
   * The constructor
   * @param led_r_pin the digital output pin that controls the red element of
   *   an RGB LED.
   * @param led_g_pin the digital output pin that controls the green element of
   *   an RGB LED.
   * @param led_b_pin the digital output pin that controls the blue element of
   *   an RGB LED.
   * @param config_path The configuration path to use to load and save the
   *   user configurable values of this device. Use BLANK if this device
   *   can not have user configurable values.
   * @param led_on_rgb The RGB color to use when the switch is in the "on"
   * state. The default value is 0x00FF00, which is RGB for "green"
   * @param led_off_rgb The RGB color to use when the switch is in the "off"
   * state. The default value is 0, RGB black, which is "off"
   */
  RgbLed(int led_r_pin, int led_g_pin, int led_b_pin, String config_path = "",
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
   * Used to set the current display state of the LED.
   * @param new_value The RGB color to display.
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