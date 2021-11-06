#include "system/pwm_output.h"

#include <Arduino.h>

#include <algorithm>

namespace sensesp {

#ifdef ESP32
// For info on frequency and resolution for ESP32, see
// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/ledc.html#ledc-api-supported-range-frequency-duty-resolution
#define CHANNEL_FREQUENCY 5000
#define CHANNEL_RESOLUTION 13
#define PWMRANGE 4095
#endif

#ifdef ESP8266
#define PWMRANGE 1023
#endif

std::map<uint8_t, int8_t> PWMOutput::channel_to_pin_;

PWMOutput::PWMOutput(int pin, int pwm_channel) {
  if (pin >= 0) {
    default_channel_ = assign_channel(pin, pwm_channel);
  }
}

void PWMOutput::set_input(float new_value, uint8_t pwm_channel) {
  if (pwm_channel == 0) {
    // Use the default channel, as zero is the SensESP default
    // input_channel for ValueConsumers
    pwm_channel = default_channel_;
  }
  set_pwm(pwm_channel, new_value);
}

int PWMOutput::assign_channel(int pin, int pwm_channel) {
  if (pwm_channel == -1) {
    // Do a search for the next available channel
    std::map<uint8_t, int8_t>::iterator it;
    pwm_channel = 0;
    do {
      pwm_channel++;
      it = channel_to_pin_.find(pwm_channel);
    } while (it != channel_to_pin_.end());
  }

  channel_to_pin_[pwm_channel] = pin;

  debugD("PWM channel %d assigned to pin %d", pwm_channel, pin);

  pinMode(pin, OUTPUT);
#ifdef ESP32
  ledcSetup(pwm_channel, CHANNEL_FREQUENCY, CHANNEL_RESOLUTION);
  ledcAttachPin(pin, pwm_channel);
#endif

  return pwm_channel;
}

void PWMOutput::set_pwm(int pwm_channel, float value) {
  std::map<uint8_t, int8_t>::iterator it;
  it = channel_to_pin_.find(pwm_channel);
  if (it != channel_to_pin_.end()) {
    int pin = it->second;
    int output_val = value * PWMRANGE;
    debugD("Outputting %d to pwm channel %d (pin %d)", output_val, pwm_channel,
           pin);

#ifdef ESP32
    uint32_t levels = pow(2, CHANNEL_RESOLUTION);
    uint32_t duty =
        ((levels - 1) / PWMRANGE) * std::min(output_val, (int)PWMRANGE);
    ledcWrite(pwm_channel, duty);
#else
    analogWrite(pin, output_val);
#endif
  } else {
    debugW("No pin assigned to channel %d. Ignoring set_pwm()", pwm_channel);
  }
}

}  // namespace sensesp
