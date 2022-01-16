#ifndef _digital_input_H_
#define _digital_input_H_

#include "sensor.h"

namespace sensesp {

/**
 * @brief DigitalInput is the base class for reading a digital GPIO pin.
 *
 * @param pin The GPIO pin to which the device is connected
 *
 * @param pin_mode Will be INPUT or INPUT_PULLUP
 *
 * */
class DigitalInput {
 public:
  DigitalInput(uint8_t pin, int pin_mode) : pin_{pin} {
    pinMode(pin_, pin_mode);
  };

 protected:
  uint8_t pin_;
};

/**
 * @brief DigitalInputState polls the state of an input pin every read_delay ms.
 *
 * It is useful for sampling the state of an input at constant time intervals
 * when you are not interested in rapid changes within those time intervals.
 *
 * You can use it for monitoring a device (such as a bilge pump) that should (or
 * shouldn't) normally be on (or off). If you use the Signal K to InfluxDb
 * Plugin and Grafana, you can easily see the frequency and duration of the
 * device being on and off.
 *
 * @param pin The GPIO pin to which the device is connected.
 *
 * @param pin_mode Will be INPUT or INPUT_PULLUP.
 *
 * @param read_delay How often you want to read the pin, in ms.
 *
 * @param config_path The path to configuring read_delay in the Config UI.
 *
 * */
class DigitalInputState : public DigitalInput, public SensorT<bool> {
 public:
  DigitalInputState(uint8_t pin, int pin_mode, int read_delay = 1000,
                    String config_path = "")
      : DigitalInput{pin, pin_mode},
        SensorT<bool>(config_path),
        read_delay_{read_delay},
        triggered_{false} {
    load_configuration();
  }

  virtual void start() override final {
    ReactESP::app->onRepeat(read_delay_, [this]() { emit(digitalRead(pin_)); });
  }

 private:
  int read_delay_;
  bool triggered_;
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
};

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
class DigitalInputCounter : public DigitalInput, public SensorT<int> {
 public:
  DigitalInputCounter(uint8_t pin, int pin_mode, int interrupt_type,
                      unsigned int read_delay, String config_path = "")
      : DigitalInput{pin, pin_mode},
        SensorT<int>(config_path),
        interrupt_type_{interrupt_type},
        read_delay_{read_delay} {
    load_configuration();
  }

  void start() override final {
    ReactESP::app->onInterrupt(pin_, interrupt_type_, [this]() { this->counter_++; });

    ReactESP::app->onRepeat(read_delay_, [this]() {
      noInterrupts();
      output = counter_;
      counter_ = 0;
      interrupts();
      notify();
    });
  }

 private:
  int interrupt_type_;
  unsigned int read_delay_;
  volatile unsigned int counter_ = 0;
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
};

/**
 * @brief DigitalInputChange provides output whenever a digital pin changes its
 * state.
 *
 * DigitalInputChange is triggered by an interrupt on the pin, so it is
 * guaranteed to catch all state changes on the pin. However, output reporting
 * occurs during the ReactESP event loop, so for extremely fast state changes
 * (with a frequency of at least several tens of kHz), it is possible that the
 * event loop can't keep up and only the latest value is processed.)
 *
 * @param pin The GPIO pin to which the device is connected
 *
 * @param pin_mode Will be INPUT or INPUT_PULLUP
 *
 * @param interrupt_type Will be RISING, FALLING, or CHANGE
 *
 * @param config_path The path to configure Sensor parameters, if any
 *
 * @see Debounce
 * */
class DigitalInputChange : public DigitalInput, public SensorT<bool> {
 public:
  DigitalInputChange(uint8_t pin, int pin_mode, int interrupt_type,
                     String config_path = "")
      : DigitalInput{pin, pin_mode},
        SensorT<bool>(config_path),
        interrupt_type_{interrupt_type},
        triggered_{true} {
    load_configuration();
    output = (bool)digitalRead(pin_);
    last_output_ = !output;  // ensure that we always send the first output
  }
  virtual void start() override final;

 private:
  int interrupt_type_;
  bool triggered_;
  bool last_output_;
  virtual void get_configuration(JsonObject& doc) override {}
  virtual bool set_configuration(const JsonObject& config) override {
    return false;
  }
};

}  // namespace sensesp

#endif
