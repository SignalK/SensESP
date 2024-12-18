#ifndef SENSESP_SYSTEM_RGB_LED_H
#define SENSESP_SYSTEM_RGB_LED_H

#include "pwm_output.h"
#include "sensesp/system/lambda_consumer.h"
#include "sensesp/ui/config_item.h"
#include "valueconsumer.h"

namespace sensesp {

/**
 * @brief A special device object that can be used to control
 * a multi-channel color rgb LED light using up to 3 digital output
 * channels. A "color" is defined using the standard 24 bit color
 * depth numbers composed of three 8 bit channels for Red, Green,
 * and Blue (see https://techterms.com/definition/rgb)
 * <p>Color LEDs can be controlled by either setting the specific
 * color to be displayed via the set(long), or by setting the
 * on/off state via set(bool). When specifying a simple on/off
 * via the the bool input, the default ON or OFF colors specified in the
 * constructor are used.
 * <p>You do not have to define all three channels (for example, if
 * you are short on output pins, or if you happen to be using a
 * two channel color led). Specify -1 for any pin for that channel
 * to be ignored. The color values used still have to adhere to
 * the 24 bit color definition however.
 */
class RgbLed : public FileSystemSaveable {
 public:
  /**
   * The constructor
   * @param led_r_pin the digital output pin that controls the red element of
   *   an RGB LED.  Specify -1 if there is no red channel connected.
   * @param led_g_pin the digital output pin that controls the green element of
   *   an RGB LED. Specify -1 if there is no green channel connected.
   * @param led_b_pin the digital output pin that controls the blue element of
   *   an RGB LED. Specify -1 if there is no blue channel connected.
   * @param config_path The configuration path to use to load and save the
   *   user configurable values of this device. Use BLANK if this device
   *   can not have user configurable values.
   * @param led_on_rgb The RGB color to use when the switch is in the "on"
   * state. The default value is 0x00FF00, which is RGB for "green"
   * @param led_off_rgb The RGB color to use when the switch is in the "off"
   * state. The default value is 0xFF0000, which is RGB red
   * @param common_anode TRUE if the led being used is a common_anode version
   *   (which inverts the PWM signal to obtain the necessary color). FALSE
   *   if the led is a common cathode
   */
  RgbLed(int led_r_pin = -1, int led_g_pin = -1, int led_b_pin = -1,
         String config_path = "", long int led_on_rgb = 0x00FF00,
         long int led_off_rgb = 0xFF0000, bool common_anode = true);

  /**
   * Used to set the current display state of the LED.
   * @param new_value The RGB color to display.
   */

  LambdaConsumer<long> rgb_consumer_ =
      LambdaConsumer<long>([this](long new_value) { set_color(new_value); });

  /**
   * Used to set the current display state of the LED with a simple on/off
   * boolean value.  Using TRUE for new_value sets the color to the ON color.
   * Using FALSE uses the OFF color.
   */
  LambdaConsumer<bool> on_off_consumer_ =
      LambdaConsumer<bool>([this](bool new_value) {
        if (new_value) {
          set_color(led_on_rgb_);
        } else {
          set_color(led_off_rgb_);
        }
      });

  virtual bool to_json(JsonObject& root) override;
  virtual bool from_json(const JsonObject& config) override;

 protected:
  uint8_t led_r_pin_;
  uint8_t led_g_pin_;
  uint8_t led_b_pin_;
  long led_on_rgb_;
  long led_off_rgb_;
  bool common_anode_;

  void set_color(long new_value);
};

const String ConfigSchema(const RgbLed& obj);
}  // namespace sensesp

#endif
