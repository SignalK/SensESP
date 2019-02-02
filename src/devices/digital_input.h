#ifndef _digital_input_H_
#define _digital_input_H_

#include "device.h"

class DigitalInput : public Device {
 public:
  DigitalInput(uint8_t pin, int pin_mode, int interrupt_type,
               String id="", String schema="");
  void enable() override final;
  bool get();
 private:
  void ICACHE_RAM_ATTR interrupt_handler();
  uint8_t pin;
  int interrupt_type;
  bool value = 0;
};

#endif
