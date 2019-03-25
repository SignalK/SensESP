#ifndef _digital_input_H_
#define _digital_input_H_

#include "device.h"

class DigitalInput : public Device {
 public:
  DigitalInput(uint8_t pin, int pin_mode, int interrupt_type,
               String id="", String schema="");
 protected:
  uint8_t pin;
  int interrupt_type;
};

// DigitalInputValue is meant to report directly the state of
// a slowly changing signal
class DigitalInputValue : public DigitalInput {
 public:
  DigitalInputValue(uint8_t pin, int pin_mode, int interrupt_type,
                    String id="", String schema="");
  void enable() override final;
  bool get();
 private:
  bool triggered = false;
  bool value = 0;
};

// DigitalInputCounter tracks rapidly changing digital inputs
class DigitalInputCounter : public DigitalInput {
 public:
  DigitalInputCounter(uint8_t pin, int pin_mode, int interrupt_type,
                      uint read_delay,
                      String id="", String schema="");
  void enable() override final;
  uint get();
 private:
  uint read_delay;
  volatile uint counter = 0;
  uint value;
};

#endif
