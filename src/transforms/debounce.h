#ifndef debounce_H
#define debounce_H

#include "transforms/transform.h"

// Developers: this isn't an ideal implementation of a templated Transform.
// See the limitations and suggestion solution in SensESP Issue #287.
// https://github.com/SignalK/SensESP/issues/287

/**
 * @brief DebounceTemplate implements debounce code for a button or switch
 * 
 * It's a passthrough transform that will output a value only if it's different
 * from the previous output, and only if it's been ms_min_delay ms since the input
 * was received, with no other input received since then.
 *
 * @tparam T The type of value being passed through Debounce.
 *
 * @param ms_min_delay The minimum amount of time that must have passed since
 * the input was received by this Transform in order for the output to occur. If
 * you are using this to debounce the output from DigitalInputChange, ms_min_delay
 * should be set at least a little bit longer than DigitalInputChange::read_delay.
 * 
 * DigitalInputChange::read_delay is 10 ms by default, and Debounce::ms_min_delay
 * is 15 ms by default. If that doesn't adequately debounce your button or switch,
 * adjust both of those values until it does.
 *
 * @param config_path The path for configuring ms_min_delay with the Config UI.
 */
template<class T>
class DebounceTemplate : public SymmetricTransform<T> {
 public:
  DebounceTemplate(int ms_min_delay = 15, String config_path = "");

  virtual void set_input(T new_value, uint8_t input_channel = 0) override;

 private:
  int ms_min_delay_;
  bool value_sent_;
  bool stable_input_;
  DelayReaction* reaction_;
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
};

typedef DebounceTemplate<bool> DebounceBool;
typedef DebounceTemplate<bool> Debounce; // for backward-compatibility with original class
typedef DebounceTemplate<int> DebounceInt;
typedef DebounceTemplate<float> DebounceFloat; // not sure this works - test it if you use it

#endif
