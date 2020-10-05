#include "digital_output.h"

#include "Arduino.h"

DigitalOutput::DigitalOutput(int pin) {
  pin_number = pin;
  pinMode(pin, OUTPUT);
}

void DigitalOutput::set_input(bool newValue, uint8_t inputChannel) {
  digitalWrite(pin_number, newValue);
}