#include "analog_input.h"

#include "../sensesp.h"

#include "Arduino.h"


void AnalogInput::update() {
  value = analogRead(A0);
  this->notify();
}

void AnalogInput::enable() {

  app.onRepeat(100, [this](){ this->update(); });
}

float AnalogInput::get() {
  return this->value;
}
