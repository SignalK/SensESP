#include "digital_output.h"

#include "Arduino.h"

namespace sensesp {

DigitalOutput::DigitalOutput(int pin) : pin_number_(pin) {
  pinMode(pin, OUTPUT);
}

void DigitalOutput::set(const bool& new_value) {
  digitalWrite(pin_number_, new_value);
  this->emit(new_value);
}

}  // namespace sensesp
