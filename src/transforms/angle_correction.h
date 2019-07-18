#ifndef _angle_correction_H_
#define _angle_correction_H_

#include "transform.h"

// Add a value to an angle input (in radians). The output
// value is wrapped to a range between [0, 2*pi[. This transform
// is useful for correcting e.g. a heading or a wind direction reading.
class AngleCorrection : public NumericTransform  {
 public:
  AngleCorrection(float offset, float min_angle=0, String config_path="");
  virtual void set_input(float input, uint8_t inputChannel = 0) override;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;

 private:
  float offset;
  float min_angle;
};

#endif
