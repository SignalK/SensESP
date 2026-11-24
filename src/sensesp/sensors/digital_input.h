#ifndef SENSESP_SENSORS_DIGITAL_INPUT_H_
#define SENSESP_SENSORS_DIGITAL_INPUT_H_

#include <elapsedMillis.h>

#include "ReactESP.h"
#include "sensesp/ui/config_item.h"
#include "sensesp_base_app.h"
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
class DigitalInputState : public DigitalInput, public Sensor<bool> {
 public:
  DigitalInputState(uint8_t pin, int pin_mode, int read_delay = 1000,
                    String config_path = "")
      : DigitalInput{pin, pin_mode},
        Sensor<bool>(config_path),
        read_delay_{read_delay} {
    load();

    repeat_event_ = event_loop()->onRepeat(read_delay_,
                           [this]() { this->emit(digitalRead(pin_)); });
  }

  virtual ~DigitalInputState() {
    if (repeat_event_ != nullptr) {
      repeat_event_->remove(event_loop());
    }
  }

  virtual bool to_json(JsonObject& root) override;
  virtual bool from_json(const JsonObject& config) override;

 private:
  int read_delay_;
  reactesp::RepeatEvent* repeat_event_ = nullptr;
};

const String ConfigSchema(const DigitalInputState& obj);

bool ConfigRequiresRestart(const DigitalInputState& obj);

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
class DigitalInputCounter : public DigitalInput, public Sensor<int> {
 public:
  DigitalInputCounter(uint8_t pin, int pin_mode, int interrupt_type,
                      unsigned int read_delay, String config_path = "")
      : DigitalInputCounter(pin, pin_mode, interrupt_type, read_delay,
                            config_path, [this]() { this->counter_ = this->counter_ + 1; }) {
    register_events();
  }

  virtual ~DigitalInputCounter() {
    if (repeat_event_ != nullptr) {
      repeat_event_->remove(event_loop());
    }
    if (isr_event_ != nullptr) {
      isr_event_->remove(event_loop().get());
    }
  }

  virtual bool to_json(JsonObject& root) override;
  virtual bool from_json(const JsonObject& config) override;

 protected:
  // load() calls the virtual from_json(). Inside a base-class constructor the
  // dynamic type is still the base, so loading there dispatches to the base
  // from_json() and silently drops fields a derived class adds. A derived class
  // with extra config therefore constructs with kNo and calls load() from its
  // own constructor body, where the full dynamic type is established.
  enum class LoadConfig { kNo, kYes };

  DigitalInputCounter(uint8_t pin, int pin_mode, int interrupt_type,
                      unsigned int read_delay, String config_path,
                      std::function<void()> interrupt_handler)
      : DigitalInputCounter(pin, pin_mode, interrupt_type, read_delay,
                            config_path, interrupt_handler, LoadConfig::kYes) {}

  DigitalInputCounter(uint8_t pin, int pin_mode, int interrupt_type,
                      unsigned int read_delay, String config_path,
                      std::function<void()> interrupt_handler,
                      LoadConfig load_config)
      : DigitalInput{pin, pin_mode},
        Sensor<int>(config_path),
        read_delay_{read_delay},
        interrupt_type_{interrupt_type},
        interrupt_handler_{interrupt_handler} {
    if (load_config == LoadConfig::kYes) {
      load();
    }
  }

  // Call after member init and after load(): registration captures read_delay_,
  // which load() may overwrite from persisted config.
  void register_events() {
    isr_event_ = event_loop()->onInterrupt(pin_, interrupt_type_, interrupt_handler_);
    repeat_event_ = event_loop()->onRepeat(read_delay_, [this]() {
      noInterrupts();
      output_ = counter_;
      counter_ = 0;
      interrupts();
      notify();
    });
  }

  unsigned int read_delay_;
  volatile unsigned int counter_ = 0;
  reactesp::RepeatEvent* repeat_event_ = nullptr;
  reactesp::ISREvent* isr_event_ = nullptr;

 private:
  int interrupt_type_;
  std::function<void()> interrupt_handler_;
};

const String ConfigSchema(const DigitalInputCounter& obj);

bool ConfigRequiresRestart(const DigitalInputCounter& obj);

/**
 * @brief DigitalInputDebounceCounter counts interrupts and reports the count
 * every read_delay ms, but ignores events that happen within
 * ignore_interval_ms.
 *
 * You can use this class if, for example, you have a noisy reed switch that
 * generates multiple interrupts every time it is actuated.
 *
 * @param pin The GPIO pin to which the device is connected
 *
 * @param pin_mode Will be INPUT or INPUT_PULLUP
 *
 * @param interrupt_type Will be RISING, FALLING, or CHANGE
 *
 * @param read_delay_ms How often you want to read the pin, in ms
 *
 * @param ignore_interval_ms Ignore events within this interval after a recorded
 * event.
 *
 * @param config_path The path to configuring read_delay in the Config UI
 */
class DigitalInputDebounceCounter : public DigitalInputCounter {
 public:
  DigitalInputDebounceCounter(uint8_t pin, int pin_mode, int interrupt_type,
                              unsigned int read_delay,
                              unsigned int ignore_interval_ms,
                              String config_path = "")
      : DigitalInputCounter(pin, pin_mode, interrupt_type, read_delay,
                            config_path, [this]() { this->handleInterrupt(); },
                            LoadConfig::kNo),
        ignore_interval_ms_{ignore_interval_ms} {
    // Load here, not in the base ctor (LoadConfig::kNo above): only now is the
    // dynamic type complete, so from_json() dispatches to this class's override
    // and ignore_interval is restored. ignore_interval_ms_ is already set, so a
    // persisted value overrides the constructor default.
    load();
    register_events();
  }

 private:
  void handleInterrupt();

  unsigned int ignore_interval_ms_;
  elapsedMillis since_last_event_;
  virtual bool to_json(JsonObject& root) override;
  virtual bool from_json(const JsonObject& config) override;
};

const String ConfigSchema(const DigitalInputDebounceCounter& obj);

bool ConfigRequiresRestart(const DigitalInputDebounceCounter& obj);

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
class DigitalInputChange : public DigitalInput, public Sensor<bool> {
 public:
  DigitalInputChange(uint8_t pin, int pin_mode, int interrupt_type,
                     String config_path = "")
      : DigitalInput{pin, pin_mode},
        Sensor<bool>(config_path),
        interrupt_type_{interrupt_type},
        triggered_{true} {
    load();
    output_ = (bool)digitalRead(pin_);
    last_output_ = !output_;  // ensure that we always send the first output_

    isr_event_ = event_loop()->onInterrupt(pin_, interrupt_type_, [this]() {
      output_ = (bool)digitalRead(pin_);
      triggered_ = true;
    });

    tick_event_ = event_loop()->onTick([this]() {
      noInterrupts();
      bool t = triggered_;
      bool val = output_;
      if (t && (val != last_output_)) {
        triggered_ = false;
        last_output_ = val;
        interrupts();
        notify();
      } else {
        interrupts();
      }
    });
  }

  virtual ~DigitalInputChange() {
    if (tick_event_ != nullptr) {
      tick_event_->remove(event_loop());
    }
    if (isr_event_ != nullptr) {
      isr_event_->remove(event_loop().get());
    }
  }

 private:
  int interrupt_type_;
  bool triggered_;
  bool last_output_;
  reactesp::TickEvent* tick_event_ = nullptr;
  reactesp::ISREvent* isr_event_ = nullptr;
  virtual bool to_json(JsonObject& doc) override { return true; }
  virtual bool from_json(const JsonObject& config) override { return true; }
};

}  // namespace sensesp

#endif
