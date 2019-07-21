#ifndef change_filter_H
#define change_filter_H

#include "transforms/transform.h"

/**
 * ChangeFilter is a numeric passthrough transform that will only
 * pass that value through if it is "sufficiently different" from
 * the last value passed through. More specifically, the absolute
 * value of the difference between the new value and the last passed
 * value has to be greater than or equal to the specified minDelta.
 */
class ChangeFilter : public NumericTransform {

    public:
        ChangeFilter(float minDelta = 1.0, String config_path="") : minDelta{minDelta},
          NumericTransform(config_path) {}

        virtual void set_input(float newValue, uint8_t inputChannel = 0) override;

        virtual JsonObject& get_configuration(JsonBuffer& buf) override;
        virtual bool set_configuration(const JsonObject& config) override;
        virtual String get_config_schema() override;

    protected:
        float minDelta;
};


#endif