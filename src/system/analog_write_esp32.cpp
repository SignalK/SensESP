#ifdef ESP32
#include "system/analog_write_esp32.h"
#include <Arduino.h>


// Code for analogWrite() simulation on ESP32 inspired by
// https://github.com/ERROPiX/ESP32_AnalogWrite

// ESP32 simulates PWM by connecting a 16 channel timer built
// into the chip to a digital output pin. This array handles
// the channel reservations for each pin the user tries
// top write to using a lazy channel reservation system.
static int8_t channel_to_pin[16] = {-1, -1, -1, -1, -1, -1, -1, -1,
                             -1, -1, -1, -1, -1, -1, -1, -1};

#define CHANNEL_FREQUENCY 5000
#define CHANNEL_RESOLUTION 13

// Translate the digital pin the user is writing to
// to the timer channel the pin is attached to. This
// function is guaranteed to succeed - if no previous
// channel reservation has been made, one is made
// automatically.
static int pin_to_channel(uint8_t pin) {
  int channel = -1;

  // Check if pin already attached to a channel
  for (uint8_t i = 0; i < 16; i++) {
    if (channel_to_pin[i] == pin) {
      channel = i;
      break;
    }
  }

  // If not, attach it to a free channel
  if (channel == -1) {
    for (uint8_t i = 0; i < 16; i++) {
      if (channel_to_pin[i] == -1) {
        channel_to_pin[i] = pin;
        channel = i;
        ledcSetup(channel, CHANNEL_FREQUENCY, CHANNEL_RESOLUTION);
        ledcAttachPin(pin, channel);
        break;
      }
    }
  }

  return channel;
}

// 
// Implement the missing analogWrite API call for ESP32...
void analogWrite(uint8_t pin, uint32_t value) {
  int channel = pin_to_channel(pin);

  // Make sure the pin was attached to a channel, if not do nothing
  if (channel != -1 && channel < 16) {
    uint32_t levels = pow(2, CHANNEL_RESOLUTION);
    uint32_t duty = ((levels - 1) / PWMRANGE) * min(value, (uint32_t)PWMRANGE);

    // write duty to LEDC
    ledcWrite(channel, duty);
  }
}

#endif
