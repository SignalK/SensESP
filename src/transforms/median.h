#ifndef _median_H_
#define _median_H_

#include <vector>

#include "transforms/transform.h"

/**
 * @brief Calculates the median of sample_size inputs.
 * 
 * Creates a sorted list of sample_size values and outputs the one
 * in the middle (i.e. element number 'sample_size / 2').
 * 
 * @param sample_size Number of values you want to take the median of.
 * 
 * @param config_path Path to configure this transform in the Config UI.
 */
class Median : public NumericTransform {
 public:
  Median(unsigned int sample_size = 10, String config_path = "");
  virtual void set_input(float input, uint8_t input_channel = 0) override;
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;

 private:
  std::vector<float> buf_;
  unsigned int sample_size_;
};

#endif
