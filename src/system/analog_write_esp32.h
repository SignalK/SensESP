#ifndef _analog_write_esp32_H_
#define _analog_write_esp32_H_

// PWM and analogWrite() compatibility module for ESP32 MUCs.

#ifdef ESP32
#include <stdint.h>

// The maximum value that can be read from or written to an analog pin
#define PWMRANGE 4095

// analogWrite() is missing from the ESP32 framework library. This function adds
// it back in, as it allows digital output pins to be modulated using
// PWM to simulate analog output in the same way an esp8266 handles it.
extern void analogWrite(uint8_t pin, uint32_t value);
#endif

#endif