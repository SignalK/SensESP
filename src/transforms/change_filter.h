#ifndef change_filter_H
#define change_filter_H

#include "transforms/transform.h"

/**
 * ChangeFilter is a numeric passthrough transform that will only
 * pass that value through if it is "sufficiently different" from
 * the last value passed through. More specifically, the absolute
 * value of the difference between the new value and the last passed
 * value has to be greater than or equal to the specified min_delta,
 * and less than or equal to the specified max_delta. If
 * the consecutive skip count ever reaches max_skips, the value
 * will be let through regardless.
 */
class ChangeFilter : public NumericTransform {
 public:
  ChangeFilter(float min_delta = 0.0, float max_delta = 9999.0,
               int max_skips = 99, String config_path = "");

  virtual void set_input(float new_value, uint8_t input_channel = 0) override;
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;

 protected:
  float min_delta;
  float max_delta;
  int max_skips;
  int skips;
};

#endif