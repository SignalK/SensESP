#include "transform.h"
#ifndef _threshold_h
#define _threshold_h

/**
 * A Transform base class that translates the value of type C into value of type P using threshold.
 * Base class for classes NumericThreshold and IntegerThreshold.
 * Arguments are minValue - minimum value for output to be inRange value.
 *               maxValue - maximum value for output to be InRange value.
 *               inRange - value if input value is in range
 *               outRange - value if input value is out of the range
 */
template <typename C, typename P>
class ThresholdTransform : public Transform<C, P> {
 public:
  ThresholdTransform(C minValue, C maxValue, P inRange, P outRange,
                     String config_path="") : Transform<C, P>(config_path) {
    this->minValue = minValue;
    this->maxValue = maxValue;
    this->inRange = inRange;
    this->outRange = outRange;
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
 * A Transform that translates float value into boolean value. User can define minValue and MaxValue to set region.
 * If value is in range transform will use inRange boolean value as output. In other cases it uses !inRange value.
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
 * A Transform that translates integer value into boolean value. User can define minValue and MaxValue to set region.
 * If value is in range transform will use inRange boolean value as output. In other cases it uses !inRange value.
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