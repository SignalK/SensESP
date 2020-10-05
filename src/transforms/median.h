#ifndef _median_H_
#define _median_H_

#include <vector>

#include "transforms/transform.h"

/**
 * Median collects a list of input values of size sample_size and
 * outputs the median value (i.e. element number sample_size / 2 of
 * a sorted sample list).
 */
class Median : public NumericTransform {
 public:
  Median(unsigned int sample_size = 10, String config_path = "");
  virtual void set_input(float input, uint8_t input_channel = 0) override;
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;

 private:
  std::vector<float> buf;
  unsigned int sample_size;
};

#endif
