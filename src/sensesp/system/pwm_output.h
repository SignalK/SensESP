#ifndef SENSESP_SYSTEM_PWM_OUTPUT_H
#define SENSESP_SYSTEM_PWM_OUTPUT_H

#include <Arduino.h>
#include <map>

#include "esp_arduino_version.h"
#include "valueconsumer.h"

namespace sensesp {

/**
 * @brief DEPRECATED. Use Android native ledcAttach and ledcWrite instead.
 *
 * <p>This class works by defining a "PWM channel". GPIO pins
 * are assigned to a channel, and then the channel is used when
 * setting the pwm value. The pwm value is a float number between
 * 0.0 and 1.0, interpreted as a percentage of the maximum duty cycle
 * possible.
 * <p>Channels can be auto-assigned to a pin or they can be declared
 * explicitly in your code. The PWM value is set either by
 * calling `set_pwm()`, or by using an instance of `PWMOutput`
 * and calling the `set()` method as defined in `ValueConsumer<>`.
 * <p>On an esp32, the "channel" corresponds to one of the timer
 * channels available on the onboard chip.  Explicit declaration
 * of a channel may or may not be necessary depending on other
 * libraries that you may be using. Unless you have a need to use an
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
  PWMOutput(int pin = -1, int pwm_channel = -1, int channel_frequency = 5000,
            int channel_resolution = 13) __attribute__((deprecated))
      : ValueConsumer<float>(),
        pwm_channel_{static_cast<uint8_t>(pwm_channel)},
        channel_frequency_{channel_frequency},
        channel_resolution_{channel_resolution},
        pwmrange_{static_cast<int>((1 << channel_resolution) - 1)} {
    if (pin >= 0) {
      pwm_channel_ = assign_channel(pin, pwm_channel);
    }
  }

  /**
   * Sets the duty cycle of the specified pwm_channel to new_value. If
   * pwm_channel is zero, the channel assigned when the PWMOutput instance
   * was instantiated will be used.
   */
  virtual void set(const float& new_value) override { set_pwm(new_value); }

 protected:
  int channel_frequency_;
  int channel_resolution_;
  int pwmrange_;

  static std::map<uint8_t, int8_t>& channel_map() {
    static std::map<uint8_t, int8_t> channel_to_pin_;
    return channel_to_pin_;
  }

  uint8_t pwm_channel_{};

  /**
   * Assigns the specified GPIO pin to the specified pwm channel.
   * @param pin the GPIO pin used for the pwm output
   * @param pwm_channel The pwm_channel to the GPIO pin is assigned
   *  to. Passing -1 specifies you would like to use
   *  the next unassigned channel available.
   * @returns The actual pwm channel assigned to the GPIO pin
   * to the pin
   */
  int assign_channel(int pin, int pwm_channel = -1) {
    if (pwm_channel == -1) {
      // Do a search for the next available channel
      std::map<uint8_t, int8_t>::iterator it;
      pwm_channel = 0;
      do {
        pwm_channel++;
        it = channel_map().find(pwm_channel);
      } while (it != channel_map().end());
    }

    channel_map()[pwm_channel] = pin;

    ESP_LOGD(__FILENAME__, "PWM channel %d assigned to pin %d", pwm_channel,
             pin);

    pinMode(pin, OUTPUT);
#if ESP_ARDUINO_VERSION_MAJOR > 2
    ledcAttachChannel(pin, channel_frequency_, channel_resolution_,
                      pwm_channel);
#else
    ledcSetup(pwm_channel, channel_frequency_, channel_resolution_);
    ledcAttachPin(pin, pwm_channel);
#endif

    return pwm_channel;
  }

  /**
   * Sets duty cycle on specified pwm channel
   * @param value A number between 0.0 and 1.0, where 1.0 is the maximum
   *   duty cycle the output pin supports.
   */
  void set_pwm(float value) {
    std::map<uint8_t, int8_t>::iterator it;
    it = channel_map().find(pwm_channel_);
    if (it != channel_map().end()) {
      int pin = it->second;
      int const output_val = value * pwmrange_;
      // ESP_LOGD(__FILENAME__, "Setting PWM channel %d to %d", pwm_channel_,
      //          output_val);
#if ESP_ARDUINO_VERSION_MAJOR > 2
      ledcWriteChannel(pwm_channel_, output_val);
#else
      ledcWrite(pwm_channel_, output_val);
#endif
    } else {
      ESP_LOGW(__FILENAME__,
               "No pin assigned to channel %d. Ignoring set_pwm()",
               pwm_channel_);
    }
  }
};

}  // namespace sensesp

#endif
