#include "digital_input.h"

#include <FunctionalInterrupt.h>

#include "sensesp.h"

DigitalInput::DigitalInput(
    uint8_t pin, int pin_mode, int interrupt_type,
    String config_path)
    : Device(config_path), pin{pin}, interrupt_type{interrupt_type} {
  pinMode(pin, pin_mode);
}

DigitalInputValue::DigitalInputValue(
    uint8_t pin, int pin_mode, int interrupt_type,
    String config_path) :
      DigitalInput{pin, pin_mode, interrupt_type, config_path},
      BooleanProducer() {}

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
    String config_path) :
      DigitalInput{pin, pin_mode, interrupt_type, config_path},
      IntegerProducer(),
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
