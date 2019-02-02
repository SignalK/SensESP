#include "digital_input.h"

#include <FunctionalInterrupt.h>

#include "sensesp.h"

DigitalInput::DigitalInput(
      uint8_t pin, int pin_mode, int interrupt_type,
      String id, String schema)
      : Device{id, schema}, pin{pin}, interrupt_type{interrupt_type} {
  pinMode(pin, pin_mode);
  Serial.println(digitalRead(pin));
}

void DigitalInput::enable() {
  app.onInterrupt(pin, interrupt_type,
                  std::bind(&DigitalInput::interrupt_handler, this));
}

void ICACHE_RAM_ATTR DigitalInput::interrupt_handler() {
  app.onDelay(0, [this](){
    this->value = digitalRead(this->pin);
    this->notify();
   });
}

bool DigitalInput::get() {
  return this->value;
}
