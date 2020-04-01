#ifndef _ads1x15_voltage_H
#define _ads1x15_voltage_H

#include "transforms/transform.h"
#include <Adafruit_ADS1015.h>

// Pass one of these in the constructor to ADS1x15Voltage() to tell which of the two chips you're working with.
enum ADS1x15CHIP_t { ADS1015chip, ADS1115chip };


/**
 * A transform that takes the output from an ADS1015 or ADS1115 analog-to-digital converter
 * (ADC) as input, and converts the value to the original voltage sensed by the ADC.
 * There are two parameters:
 * - chip, which is either 0 (representing the ADS1015) or 1 (representing the ADS1115). Default is 1.
 * - gain, which is the value of the GAIN setting used by the firmware on the chip. It defaults to 
     GAIN_TWOTHIRDS, which is also the default GAIN setting for both chips. Valid values are from
     the ADAfruit_ADS1015 library:
        GAIN_TWOTHIRDS = +/-6.144V range
        GAIN_ONE = +/-4.096V range
        GAIN_TWO = +/-2.048V range
        GAIN_FOUR = +/-1.024V range
        GAIN_EIGHT = +/-0.512V range
        GAIN_SIXTEEN = +/-0.256V range
 *   
 * It's up to you to set the proper chip and gain to match what you've set in ADS1015() or 
 * ADS1115() - they are not automatically detected.
 */
class ADS1x15Voltage : public NumericTransform {

    public:
        ADS1x15Voltage(ADS1x15CHIP_t chip = ADS1115chip, adsGain_t gain = GAIN_TWOTHIRDS);

        virtual void set_input(float input, uint8_t inputChannel = 0) override;

    protected:
        ADS1x15CHIP_t chip;
        adsGain_t gain;

};

#endif
