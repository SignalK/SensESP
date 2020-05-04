#ifndef _timer_H_
#define _timer_H_

#include "transforms/transform.h"

/**
 * Timer takes a boolean as input and counts how many seconds
 * the boolean is active. resetPeriod is the amount of time
 * that has to pass for a reset of the value.
 */
class Timer : public Transform<bool, int> {

 public:
  Timer(unsigned long resetPeriod = 0, bool defaultState = false, String config_path="");
  virtual void set_input(bool input, uint8_t inputChannel = 0) override;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;

 private:
  uint8_t time;
  bool last_value;
  unsigned long init_time;
  unsigned long end_time;
  bool add;
  static long unsigned int lastSampleTime;
  unsigned long resetPeriod;
  bool defaultState;
};

#endif
