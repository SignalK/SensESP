#include "sensesp.h"

#include "pwm_output.h"

#include <algorithm>

namespace sensesp {

// For info on frequency and resolution for ESP32, see
// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/ledc.html#ledc-api-supported-range-frequency-duty-resolution

std::map<uint8_t, int8_t> PWMOutput::channel_to_pin_;

PWMOutput::PWMOutput(int pin, int pwm_channel, int channel_frequency,
                     int channel_resolution)
    : ValueConsumer<float>(),
      pwm_channel_{static_cast<uint8_t>(pwm_channel)},
      channel_frequency_{channel_frequency},
      channel_resolution_{channel_resolution},
      pwmrange_{static_cast<int>(pow(2, channel_resolution) - 1)} {
  if (pin >= 0) {
    pwm_channel_ = assign_channel(pin, pwm_channel);
  }
}

void PWMOutput::set(const float& new_value) { set_pwm(new_value); }

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

  ESP_LOGD(__FILENAME__, "PWM channel %d assigned to pin %d", pwm_channel, pin);

  pinMode(pin, OUTPUT);
  ledcSetup(pwm_channel, channel_frequency_, channel_resolution_);
  ledcAttachPin(pin, pwm_channel);

  return pwm_channel;
}

void PWMOutput::set_pwm(float value) {
  std::map<uint8_t, int8_t>::iterator it;
  it = channel_to_pin_.find(pwm_channel_);
  if (it != channel_to_pin_.end()) {
    int pin = it->second;
    int const output_val = value * pwmrange_;
    ESP_LOGD(__FILENAME__, "Outputting %d to pwm channel %d (pin %d)",
             output_val, pwm_channel_, pin);
    ledcWrite(pwm_channel_, output_val);
  } else {
    ESP_LOGW(__FILENAME__, "No pin assigned to channel %d. Ignoring set_pwm()",
             pwm_channel_);
  }
}

}  // namespace sensesp
