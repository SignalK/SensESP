#include "digital_input.h"

#include <FunctionalInterrupt.h>

#include "sensesp.h"

DigitalInput::DigitalInput(
    uint8_t pin, int pin_mode, int interrupt_type,
    String id, String schema)
    : Device{id, schema}, pin{pin}, interrupt_type{interrupt_type} {
  pinMode(pin, pin_mode);
}

DigitalInputValue::DigitalInputValue(
    uint8_t pin, int pin_mode, int interrupt_type,
    String id, String schema, uint8_t valueIndex) :
      DigitalInput{pin, pin_mode, interrupt_type, id, schema},
      BooleanProducer(valueIndex) {}

void DigitalInputValue::enable() {
  app.onInterrupt(
    pin, interrupt_type,
    [this](){
      output = digitalRead(pin);
      triggered = true;
    });
  app.onTick(
    [this](){
      if (triggered) {
        triggered = false;
        notify();
      }
    }
  );
}


DigitalInputCounter::DigitalInputCounter(
    uint8_t pin, int pin_mode, int interrupt_type,
    uint read_delay,
    String id, String schema, uint8_t valueIndex) :
      DigitalInput{pin, pin_mode, interrupt_type, id, schema},
      IntegerProducer(valueIndex),
      read_delay{read_delay} {}

void DigitalInputCounter::enable() {
  app.onInterrupt(pin, interrupt_type,
                  [this](){
    this->counter++;
  });
  
  app.onRepeat(read_delay, [this](){
    noInterrupts();
    output = counter;
    counter = 0;
    interrupts();
    notify();
  });
}
