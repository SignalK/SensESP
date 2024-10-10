#ifndef SENSESP_TRANSFORMS_PRESS_REPEATER_H_
#define SENSESP_TRANSFORMS_PRESS_REPEATER_H_

#include <elapsedMillis.h>

#include "sensesp/ui/config_item.h"
#include "sensesp/sensors/digital_input.h"
#include "sensesp/system/valueconsumer.h"
#include "transform.h"

namespace sensesp {

/**
 * @brief A transform that takes boolean inputs and adds
 * button behaviors familiar to many device end users.
 * DEPRECATED: use AceButton for button inputs.
 *
 * It emits a value only when the state of the input changes
 * (i.e. when the input changes from TRUE to FALSE, and vice versa).
 * In addition, if the input remains TRUE longer than repeat_start_interval
 * milliseconds, it will emit TRUE once again, and then again every
 * repeat_interval milliseconds until the input returns to FALSE.
 *
 * <p>An example use case would be a DigitalInput connected to a button
 * that represents the "Volume Up" or "Volume Down" of a sound system.
 *
 * <p>As a convenience for wiring up to DigitalInputValue and other
 * producers that emit integers, PressRepeater can also consume
 * integer values. As long as the integer value coming in does not
 * match integer_false, PressRepeater will act as if TRUE was passed to it.
 *
 * @param config_path The path to configure this transform in Config UI
 *
 * @param integer_false Whatever integer value (of your input) you want
 * to be the same as boolean "false". Default is 0.
 *
 * @param repeat_start_interval How long the input must be TRUE before
 * the first repeated output, in ms. Default is 1500.
 *
 * @param repeat_interval How often to repeat the repeated output, in ms.
 * The default is 250.
 */
class PressRepeater : public BooleanTransform {
 public:
  PressRepeater(const String& config_path = "", int integer_false = 0,
                int repeat_start_interval = 1500, int repeat_interval = 250);

  virtual void set(const bool& new_value) override;

  virtual bool to_json(JsonObject& root) override;
  virtual bool from_json(const JsonObject& config) override;

 protected:
  int integer_false_;
  int repeat_start_interval_;
  int repeat_interval_;
  elapsedMillis last_value_sent_;
  bool pushed_;
  bool repeating_;
};

const String ConfigSchema(const PressRepeater& obj);

}  // namespace sensesp
#endif
