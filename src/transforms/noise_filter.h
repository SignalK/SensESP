#ifndef noise_filter_H
#define noise_filter_H

#include "transforms/transform.h"

/**
 * NoiseFilter is a numeric passthrough transform that will only
 * pass that value through if it is "sufficiently close to" 
 * the last value passed through. More specifically, the absolute
 * value of the difference between the new value and the last passed
 * value has to be less than or equal to the specified maxDelta. If
 * the consecutive skip count ever reaches maxSkips, the value
 * will be let through. 
 */
class NoiseFilter : public NumericTransform {

    public:
        NoiseFilter(float maxDelta = 1.0, int maxSkips = 20, String config_path="");

        virtual void set_input(float newValue, uint8_t inputChannel = 0) override;

        virtual JsonObject& get_configuration(JsonBuffer& buf) override;
        virtual bool set_configuration(const JsonObject& config) override;
        virtual String get_config_schema() override;

    protected:
        float maxDelta;
        int maxSkips;
        int skips;
};


#endif