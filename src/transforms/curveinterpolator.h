#ifndef _CurveInterpolator_H_
#define _CurveInterpolator_H_

#include "transforms/transform.h"

/**
 * CurveInterpolator uses a collection of input/output samples that approximate a non-linear curve.
 * The output is the linear interpolation between the two sample points that the input falls
 * between. It is used primarily for non-linear analog gauges such as temperature gauges and
 * oil pressure gauges, which get their input from analog sensors that are variable resistors.
 */

class CurveInterpolator : public NumericTransform {

 public:
    class Sample {
       public:
         float input;
         float output;

         Sample();
         Sample(float input, float output);
         Sample(JsonObject& obj);

         friend bool operator<(const Sample& lhs, const Sample& rhs) { return lhs.input < rhs.input; }
    };

 public:
   CurveInterpolator(std::set<Sample>* defaults = NULL, String config_path="");

   // Set and retrieve the transformed value
   void set_input(float input, uint8_t inputChannel = 0) override final;


   // For reading and writing the configuration of this transformation
   virtual JsonObject& get_configuration(JsonBuffer& buf) override;
   virtual bool set_configuration(const JsonObject& config) override;
   virtual String get_config_schema() override;
   
   // For manually adding sample points
   void clearSamples();
   void addSample(const Sample& newSample);

 private:
    std::set<Sample> samples;

};

#endif
