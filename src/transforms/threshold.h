#include "transform.h"
#ifndef _threshold_h
#define _threshold_h

/**
 * A Transform base class that translates the value of type C into value of type P.
 * Base class for classes NumericThreshold and IntegerThreshold.
 * Arguments are min_value - minimum value of input for output to be the value of in_range.
 *               max_value - maximum value of input for output to be the value of in_range.
 *               in_range - output value if input value is in range
 *               out_range - output value if input value is out of the range
 */
template <typename C, typename P>
class ThresholdTransform : public Transform<C, P> {
 public:
  ThresholdTransform(C min_value, C max_value, P in_range, P out_range,
           String config_path="") : Transform<C, P>(config_path), min_value{min_value},
           max_value{max_value}, in_range{in_range}, out_range{out_range}  {
    this->load_configuration();
};
  virtual void set_input(C new_value, uint8_t input_channel = 0) override;
 protected:
  C min_value;
  C max_value;
  P in_range;
  P out_range;
};

/**
 * A Transform that translates a float value into a boolean value. min_value and MaxValue set a range.
 * If input value is in the range, the output will be the value of in_range. Otherwise, it will be !in_range.
 */
class NumericThreshold : public ThresholdTransform<float, bool>
{
    public:
      NumericThreshold(float min_value, float max_value, bool in_range = true,
                     String config_path="") :
                      ThresholdTransform<float, bool>(min_value, max_value, in_range, !in_range, config_path)
                      {}

    virtual void get_configuration(JsonObject& doc) override;
    virtual bool set_configuration(const JsonObject& config) override;
    virtual String get_config_schema() override;
};

/**
 * A Transform that translates an integer value into a boolean value. min_value and MaxValue set a range.
 * If input value is in the range, the output will be the value of in_range. Otherwise, it will be !in_range.
 */
class IntegerThreshold : public ThresholdTransform<int, bool>
{
    public:
      IntegerThreshold(int min_value, int max_value, bool in_range = true,
                     String config_path="") :
                      ThresholdTransform<int, bool>(min_value, max_value, in_range, !in_range, config_path)
                      {}

    virtual void get_configuration(JsonObject& doc) override;
    virtual bool set_configuration(const JsonObject& config) override;
    virtual String get_config_schema() override;
};
#endif