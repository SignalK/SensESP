#ifndef _counter_H_
#define _counter_H_

#include "transforms/transform.h"

/**
 * Counter takes a boolean as input and counts how many times
 * the boolean is switching. resetPeriod is the amount of time
 * that has to pass for a reset of the value.
 */
class Counter : public Transform<bool, int> {

 public:
  Counter(unsigned long resetPeriod = 0, bool defaultState = false, String config_path="");
  virtual void set_input(bool input, uint8_t inputChannel = 0) override;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;

 private:
  uint8_t count;
  bool last_value;
  static long unsigned int lastSampleTime;
  unsigned long resetPeriod;
  bool defaultState;
};

#endif
