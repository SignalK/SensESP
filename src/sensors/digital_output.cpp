#include "digital_output.h"
#include "Arduino.h"

DigitalOutput::DigitalOutput(int pin)
{
    pinNumber = pin;
    pinMode(pin, OUTPUT);
}

void DigitalOutput::set_input(bool newValue, uint8_t inputChannel)
{
    digitalWrite(pinNumber, newValue);
}