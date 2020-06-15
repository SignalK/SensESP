#ifndef _digital_input_H_
#define _digital_input_H_

#include "sensor.h"

class DigitalInput : public Sensor {
 public:
  DigitalInput(uint8_t pin, int pin_mode, int interrupt_type,
               String config_path="");

 protected:
  uint8_t pin;
  int interrupt_type;
};


// DigitalInputValue is meant to report directly the state of
// a slowly changing signal
class DigitalInputValue : public DigitalInput, public IntegerProducer {
 public:
  DigitalInputValue(uint8_t pin, int pin_mode, int interrupt_type, int read_delay = 1000,
                    String config_path="");

  virtual void enable() override final;

 private:
  bool triggered = false;
  int read_delay;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
};

// DigitalInputCounter tracks rapidly changing digital inputs
class DigitalInputCounter : public DigitalInput, public IntegerProducer {
 public:
  DigitalInputCounter(uint8_t pin, int pin_mode, int interrupt_type,
                      uint read_delay,
                      String config_path="");

  void enable() override final;

 private:
  uint read_delay;
  volatile uint counter = 0;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
};

#endif
