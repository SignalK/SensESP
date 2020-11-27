#include "digital_output.h"

#include "Arduino.h"

DigitalOutput::DigitalOutput(int pin) {
  pin_number = pin;
  pinMode(pin, OUTPUT);
}

void DigitalOutput::set_input(bool new_value, uint8_t inputChannel) {
  digitalWrite(pin_number, new_value);
  this->emit(new_value);
}
