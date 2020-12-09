#ifndef debounce_H
#define debounce_H

#include "transforms/transform.h"
#include <elapsedMillis.h>

// Developers: this isn't an ideal implementation of a templated Transform.
// See the limitations and suggestion solution in SensESP Issue #287.
// https://github.com/SignalK/SensESP/issues/287

/**
 * @brief DebounceTemplate implements debounce code for a button or switch
 * 
 * It's a passthrough transform that will output a value immediately on the
 * initial input, and then only when the time between inputs is greater than
 * ms_min_delay milliseconds. If you're monitoring the button press and the button
 * release, you need to make sure that the time between the stabilization of the
 * the button press and the beginning of the button release is longer than ms_min_delay.
 *
 * @tparam T The type of value being passed through Debounce.
 *
 * @param ms_min_delay The minimum amount of time that must have passed since
 * the previous input in order for the output to occur. This needs to be larger than the
 * number of milliseconds between the successive bounces in a bouncy signal. For example,
 * if your button bounces up and down every 5 milliseconds between the time you start the
 * button press and the time all the bouncing stops, then ms_min_delay needs to be larger
 * than 5. To be safe, make it at least twice the length of the longest bouncer interval
 * you know your button might generate. The default value for this class is 20 ms.
 *
 * @param config_path The path for configuring ms_min_delay with the Config UI.
 */
template<class T>
class DebounceTemplate : public SymmetricTransform<T> {
 public:
  DebounceTemplate(int ms_min_delay = 20, String config_path = "");

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
typedef DebounceTemplate<float> DebounceFloat;
typedef DebounceTemplate<String> DebounceString;

#endif
