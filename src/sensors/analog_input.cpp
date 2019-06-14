#include "analog_input.h"

#include "Arduino.h"

#include "sensesp.h"

void AnalogInput::update() {
  output = analogRead(A0);
  this->notify();
}

void AnalogInput::enable() {
  app.onRepeat(100, [this](){ this->update(); });
}
