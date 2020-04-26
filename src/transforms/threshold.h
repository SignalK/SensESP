#include "transform.h"
#ifndef _threshold_h
#define _threshold_h
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


class NumericThreshold : public ThresholdTransform<float, bool>
{
    public:
      NumericThreshold(float minValue, float maxValue, bool inRange,
                     String config_path="") :
                      ThresholdTransform<float, bool>(minValue, maxValue, inRange, !inRange, config_path)
                      {}

    virtual JsonObject& get_configuration(JsonBuffer& buf) override;
    virtual bool set_configuration(const JsonObject& config) override;
    virtual String get_config_schema() override;
};

class IntegerThreshold : public ThresholdTransform<int, bool>
{
    public:
      IntegerThreshold(int minValue, int maxValue, bool inRange,
                     String config_path="") :
                      ThresholdTransform<int, bool>(minValue, maxValue, inRange, !inRange, config_path)
                      {}

    virtual JsonObject& get_configuration(JsonBuffer& buf) override;
    virtual bool set_configuration(const JsonObject& config) override;
    virtual String get_config_schema() override;
};
#endif