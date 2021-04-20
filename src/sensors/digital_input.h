#ifndef _digital_input_H_
#define _digital_input_H_

#include "sensor.h"

 /**
 * @brief DigitalInput is the base class for reading a digital GPIO pin.
 *
 * @param pin The GPIO pin to which the device is connected
 * 
 * @param pin_mode Will be INPUT or INPUT_PULLUP
 * 
 * @param interrupt_type Will be RISING, FALLING, or CHANGE
 * 
 * @param config_path The path to configuring read_delay in the Config UI
 * 
 * */
class DigitalInput : public Sensor {
 public:
  DigitalInput(uint8_t pin, int pin_mode, 
               String config_path = "");

 protected:
  uint8_t pin_;
};


 /**
 * @brief DigitalInputState reports the state of a digital pin every read_delay ms.
 * 
 * Formerly called DigitalInputValue.
 *
 * It's useful for monitoring a device (such as a bilge pump) that should (or
 * shouldn't) normally be on (or off). If you use the Signal K to InfluxDb Plugin
 * and Grafana, you can easily see the frequency and duration of the device being
 * on and off.
 *
 * @param pin The GPIO pin to which the device is connected.
 * 
 * @param pin_mode Will be INPUT or INPUT_PULLUP.
 * 
 * @param interrupt_type Will be RISING, FALLING, or CHANGE, but it doesn't matter
 * which one you use, as this Class doesn't use an interrupt.
 * 
 * @param read_delay How often you want to read the pin, in ms.
 * 
 * @param config_path The path to configuring read_delay in the Config UI.
 * 
 * */
class DigitalInputState : public DigitalInput, public BooleanProducer {
 public:

  DigitalInputState(uint8_t pin, int pin_mode, int read_delay = 1000, String config_path = "");

  virtual void enable() override final;

 private:
  int read_delay_;
  bool triggered_;
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
};

// FIXME: Uncomment the following once the PIO Xtensa toolchain is updated
// [[deprecated("Use DigitalInputState instead.")]]
typedef DigitalInputState DigitalInputValue;


 /**
 * @brief DigitalInputCounter counts interrupts and reports the count every
 * read_delay ms.
 *
 * A typical use case is to count the revolutions of something, such as an
 * engine, to determine RPM. See /examples/rpm_counter.cpp
 *
 * @param pin The GPIO pin to which the device is connected
 * 
 * @param pin_mode Will be INPUT or INPUT_PULLUP
 * 
 * @param interrupt_type Will be RISING, FALLING, or CHANGE
 * 
 * @param read_delay How often you want to read the pin, in ms
 * 
 * @param config_path The path to configuring read_delay in the Config UI
 * 
 * */
class DigitalInputCounter : public DigitalInput, public IntegerProducer {
 public:
  DigitalInputCounter(uint8_t pin, int pin_mode, int interrupt_type,
                      uint read_delay, String config_path = "");

  void enable() override final;

 private:
  uint read_delay_;
  int interrupt_type_;
  volatile uint counter_ = 0;
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
};


 /**
 * @brief DigitalInputChange is meant to report when a digital pin changes its state.
 *
 * It's intended for monitoring a button or other type of relatively slow changing switch.
 * Because physical switches like buttons can be somewhat "noisy", you may want to connect
 * the output of this Sensor to the Debounce Transform, and try different combinations of
 * read_delay for this Sensor and ms_min_delay for Debounce to get the "cleanest" output
 * from your button or other switch. (Make Debounce::ms_min_delay at least a little longer
 * than DigitalInputChange::read_delay)
 *
 * @param pin The GPIO pin to which the device is connected
 * 
 * @param pin_mode Will be INPUT or INPUT_PULLUP
 * 
 * @param interrupt_type Will be RISING, FALLING, or CHANGE
 * 
 * @param read_delay How often you want to read the state of the pin
 * 
 * @param config_path The path to configure read_delay_ in the Config UI
 * 
 * @see Debounce
 * */
class DigitalInputChange : public DigitalInput, public BooleanProducer {
 public:
  DigitalInputChange(uint8_t pin, int pin_mode, int interrupt_type, uint read_delay = 10,
                                         String config_path = "");
  virtual void enable() override final;
 
 private:
  uint read_delay_;
  int interrupt_type_;
  bool triggered_;
  bool last_output_;
  bool value_sent_;
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
};

#endif
