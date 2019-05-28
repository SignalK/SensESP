#ifndef _Interpolate_H_
#define _Interpolate_H_

#include "transforms/transform.h"


/**
 * Interpolate uses a collection of input/output samples that approximate a non-linear curve.
 * The output is the linear interpolation between the two sample points the input falls
 * between.  It is used primarily for non-linear analog gauges such as temperature gauges using
 * Thermocouples 
 */
class Interpolate : public OneToOneTransform<float> {

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
   Interpolate(String sk_path, std::set<Sample>* defaults = NULL, String id="", String schema="");

   // Set and retrieve the transformed value
   void set_input(float input, uint8_t inputChannel = 0) override final;


   // For outputting the results as SignalK
   String as_json() override;

   // For reading and writing the configuration of this transformation
   virtual JsonObject& get_configuration(JsonBuffer& buf) override;
   virtual bool set_configuration(const JsonObject& config) override;

   // For manually adding sample points
   void clearSamples();
   void addSample(const Sample& newSample);

 private:
    std::set<Sample> samples;

};

#endif
