#ifndef _pwm_output_H_
#define _pwm_output_H_

#include <map>

#include "system/valueconsumer.h"

/**
 * @brief PWMOutput provides a cross platform mechanism for generating
 * Pulse Width Modulation signals over one or more GPIO pins
 * on the MCU.
 * <p>This class works by defining a "PWM channel". GPIO pins
 * are assigned to a channel, and then the channel is used when
 * setting the pwm value. The pwm value is a float number between
 * 0.0 and 1.0, interpreted as a percentage of the maximum duty cycle
 * possible.
 * <p>Channels can be auto-assigned to a pin or they can be declared
 * explicitly in your code. The PWM value is set either by
 * calling `set_pwm()`, or by using an instance of `PWMOutput`
 * and calling the `set_input()` method as defined in `ValueConsumer<>`.
 * <p>On an esp32, the "channel" corresponds to one of the timer
 * channels available on the onboard chip.  Explicit declaration
 * of a channel may or may not be necessary depending on other
 * libraries that you may be using. On an esp8266, the channel
 * is an artificial concept. Unless you have a need to use an
 * explicit channel (e.g. if some other software is already using
 * a particular esp32 timer channel), the channel auto-assign
 * feature is the easiest to use. In either event, using this
 * channel number allows the same code to work on either platform.
 */
class PWMOutput : public ValueConsumer<float> {
 public:
  /**
   * Creates an instance of a PWMOutput ValueConsumer.
   * @param pin The GPIO pin used to output the pwm signal. Pass
   *   -1 if this instance does not manage a specific pin. Using
   *   -1 is only valid if channel assignments have been explicitly
   *   made by calling `PWMOutput::assign_channel()`
   * @param pwm_channel The pwm channel the GPIO pin is to be assigned
   *   to.  Pass -1 if you would like the system to auto-select the
   *   next unassigned pin.
   */
  PWMOutput(int pin = -1, int pwm_channel = -1);

  /**
   * Sets the duty cycle of the specified pwm_channel to new_value. If
   * pwm_channel is zero, the channel assigned when the PWMOutput instance
   * was instantiated will be used.
   */
  virtual void set_input(float new_value, uint8_t pwm_channel = 0) override;

  /**
   * Assigns the specified GPIO pin to the specified pwm channel.
   * @param pin the GPIO pin used for the pwm output
   * @param pwm_channel The pwm_channel to the GPIO pin is assigned
   *  to. Passing -1 specifies you would like to use
   *  the next unassigned channel available.
   * @returns The actual pwm channel assigned to the GPIO pin
   * to the pin
   */
  static int assign_channel(int pin, int pwm_channel = -1);

  /**
   * Sets duty cycle on specified pwm channel
   * @param pwm_channel The pwm_channel assigned to the GPIO pin
   * @param value A number between 0.0 and 1.0, where 1.0 is the maximum
   *   duty cycle the output pin supports.
   */
  static void set_pwm(int pwm_channel, float value);

 protected:
  static std::map<uint8_t, int8_t> channel_to_pin_;
  uint8_t default_channel_;
};

#endif