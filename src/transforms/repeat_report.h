#ifndef _repeat_report_H_
#define _repeat_report_H_

#include <elapsedMillis.h>

#include "transforms/transform.h"

/**
 * @brief Ensures that values that do not change frequently are still
 * reported at a specified maximum silence interval. If the value has not
 * changed in max_silence_interval milliseconds, the current value is emmitted
 * again.
 * 
 * @param max_silence_interval Maximum time, in ms, before the previous value
 * is emitted again. Default is 15000 (15 seconds).
 * 
 * @param config_path Path to configure this transform in the Config UI.
 */
template <typename T>
class RepeatReport : public SymmetricTransform<T> {
 public:
  RepeatReport(long max_silence_interval = 15000, String config_path = "")
      : SymmetricTransform<T>(config_path),
        max_silence_interval_{max_silence_interval} {
    this->load_configuration();
  }

  virtual void set_input(T input, uint8_t inputChannel = 0) override;
  virtual void enable() override;
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;

 private:
  long max_silence_interval_;
  elapsedMillis last_update_interval_;
};

#endif