#ifndef _voltage_multiplier_H
#define _voltage_multiplier_H

#include "transforms/transform.h"


/**
 * A transform that does the opposite of what a physical voltage divider circuit does.
 * Used to convert the voltage that comes out of a voltage divider back into the 
 * original voltage that went into the voltage divider.
 * Use case: measuring a ~12V DC source with an analog-to-digital converter (ADC) that can take
 * only 5V. You use a physical voltage divider circuit to reduce from ~12V down to a range
 * that's no higher than 5V. After you read the output from the ADC and convert it into
 * voltage, you have to reverse the effect of the voltage divider circuit to know the original
 * source voltage.
 * Two parameters:
 * - R1 = the value, in ohms, of R1 in your voltage divider circuit
 * - R2 = value of R2 in the circuit
 * For maximum accuracy, you should measure the actual ohms value of your resistors, rather
 * than just assuming they are as labeled.
 */
class VoltageMultiplier : public NumericTransform {

    public:
        VoltageMultiplier(uint16_t R1, uint16_t R2, String config_path = "");

        virtual void set_input(float input, uint8_t inputChannel = 0);

    private:
        uint16_t R1;
        uint16_t R2;
        virtual JsonObject& get_configuration(JsonBuffer& buf) override;
        virtual bool set_configuration(const JsonObject& config) override;
        virtual String get_config_schema() override;
};

#endif
