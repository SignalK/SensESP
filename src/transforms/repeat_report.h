#ifndef _repeat_report_H_
#define _repeat_report_H_

#include "transforms/transform.h"

/**
 * RepeatReport ensures that values that do not change frequently are still reported at
 * at a specified maximum silence interval. If the value has not changed in max_silence_interval 
 * milliseconds, the current value is emmitted again, 
 */
template <typename T>
class RepeatReport : public SymmetricTransform<T> {
 public:
  RepeatReport(long max_silence_interval = 15000, String config_path = "") :
     SymmetricTransform<T>(config_path),
     max_silence_interval{max_silence_interval} {
     this->load_configuration();
  }

  // The followig methods moved to CPP file to avoid excessive inline code generation...
  virtual void set_input(T input, uint8_t inputChannel = 0) override;
  virtual void enable() override;
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;

 private:
  long max_silence_interval;
  uint last_update = 0;

}; 

#endif