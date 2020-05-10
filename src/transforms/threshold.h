#include "transform.h"
#ifndef _threshold_h
#define _threshold_h

/**
 * A Transform base class that translates the value of type C into value of type P.
 * Base class for classes NumericThreshold and IntegerThreshold.
 * Arguments are minValue - minimum value of input for output to be the value of inRange.
 *               maxValue - maximum value of input for output to be the value of inRange.
 *               inRange - output value if input value is in range
 *               outRange - output value if input value is out of the range
 */
template <typename C, typename P>
class ThresholdTransform : public Transform<C, P> {
 public:
  ThresholdTransform(C minValue, C maxValue, P inRange, P outRange,
           String config_path="") : Transform<C, P>(config_path), minValue{minValue},
           maxValue{maxValue}, inRange{inRange}, outRange{outRange}  {
    Enable::className = "ThresholdTransform";
    this->load_configuration();
};
  virtual void set_input(C newValue, uint8_t inputChannel = 0) override;
 protected:
  C minValue;
  C maxValue;
  P inRange;
  P outRange;
};

/**
 * A Transform that translates a float value into a boolean value. minValue and MaxValue set a range.
 * If input value is in the range, the output will be the value of inRange. Otherwise, it will be !inRange.
 */
class NumericThreshold : public ThresholdTransform<float, bool>
{
    public:
      NumericThreshold(float minValue, float maxValue, bool inRange = true,
                     String config_path="") :
                      ThresholdTransform<float, bool>(minValue, maxValue, inRange, !inRange, config_path)
                      {}

    virtual JsonObject& get_configuration(JsonBuffer& buf) override;
    virtual bool set_configuration(const JsonObject& config) override;
    virtual String get_config_schema() override;
};

/**
 * A Transform that translates an integer value into a boolean value. minValue and MaxValue set a range.
 * If input value is in the range, the output will be the value of inRange. Otherwise, it will be !inRange.
 */
class IntegerThreshold : public ThresholdTransform<int, bool>
{
    public:
      IntegerThreshold(int minValue, int maxValue, bool inRange = true,
                     String config_path="") :
                      ThresholdTransform<int, bool>(minValue, maxValue, inRange, !inRange, config_path)
                      {}

    virtual JsonObject& get_configuration(JsonBuffer& buf) override;
    virtual bool set_configuration(const JsonObject& config) override;
    virtual String get_config_schema() override;
};
#endif