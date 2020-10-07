#include "transform.h"
#ifndef _hysteresis_h
#define _hysteresis_h

/**
 * A Transform base class that translates the value of type C into value of type P.
 * Base class for classes NumericHysteresis and IntegerHysteresis.
 * Arguments are minValue - minimum value of the hysteresis range.
 *               maxValue - maximum value of the hysteresis range.
 *               belowRange - output value if input is smaller than the min value of the range.
 *               aboveRange - output value if input is greater than the max value of the range.
 */
template <typename C, typename P>
class HysteresisTransform : public Transform<C, P> {
 public:
  HysteresisTransform(C minValue, C maxValue, P belowRange, P aboveRange,
           String config_path="") : Transform<C, P>(config_path), minValue{minValue},
           maxValue{maxValue}, aboveRange{aboveRange}, belowRange{belowRange}  {
    Enable::className = "HysteresisTransform";
    this->load_configuration();
};
  virtual void set_input(C newValue, uint8_t inputChannel = 0) override;
 protected:
  C minValue;
  C maxValue;
  P aboveRange;
  P belowRange;
};

/**
 * A Transform that translates a float value into a boolean value. minValue and MaxValue set the upper 
 * and lower boundaries of the hysteresis range. 
 * No output state changes occur within the range. 
 * If input value is outside of the hysteresis range to the minValue side, the output will be the value of 
 * belowRange.
 * If input value is outside of the hysteresis range to the maxValue side, the output will be the value of 
 * !belowRange.
 */
class NumericHysteresis : public HysteresisTransform<float, bool>
{
    public:
      NumericHysteresis(float minValue, float maxValue, bool belowRange = false,
                     String config_path="") :
                      HysteresisTransform<float, bool>(minValue, maxValue, belowRange, !belowRange, config_path)
                      {}

    virtual void get_configuration(JsonObject& doc) override;
    virtual bool set_configuration(const JsonObject& config) override;
    virtual String get_config_schema() override;
};

/**
 * A Transform that translates a integer value into a boolean value. minValue and MaxValue set the upper 
 * and lower boundaries of the hysteresis range. 
 * No output state changes occur while the input is within the range. 
 * If input value is outside of the hysteresis range to the minValue side, the output will be the value of 
 * belowRange.
 * If input value is outside of the hysteresis range to the maxValue side, the output will be the value of 
 * !belowRange.
 */
class IntegerHysteresis : public HysteresisTransform<int, bool>
{
    public:
      IntegerHysteresis(int minValue, int maxValue, bool belowRange = false,
                     String config_path="") :
                      HysteresisTransform<int, bool>(minValue, maxValue, belowRange, !belowRange, config_path)
                      {}

    virtual void get_configuration(JsonObject& doc) override;
    virtual bool set_configuration(const JsonObject& config) override;
    virtual String get_config_schema() override;
};
#endif