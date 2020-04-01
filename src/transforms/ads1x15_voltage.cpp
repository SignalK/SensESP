#include "ads1x15_voltage.h"

ADS1x15Voltage::ADS1x15Voltage(ADS1x15CHIP_t chip, adsGain_t gain) :
       NumericTransform(), chip{chip}, gain{gain} {
     className = "ADS1x15Voltage";
}

void ADS1x15Voltage::set_input(float input, uint8_t inputChannel) {
    if (chip == ADS1015chip) {
      switch (gain) { 
        case GAIN_TWOTHIRDS: output = input * 0.003;
                break;
        case GAIN_ONE: output = input * 0.002;
                break;
        case GAIN_TWO: output = input * 0.001;
                break;
        case GAIN_FOUR: output = input * 0.0005; 
                break; 
        case GAIN_EIGHT: output = input * 0.00025; 
                 break; 
        case GAIN_SIXTEEN: output = input * 0.000125; 
                break; 
        default: debugE("FATAL: invalid GAIN parameter.");  
      }
    }
    else if (chip == ADS1115chip) {
       switch (gain) { 
        case GAIN_TWOTHIRDS: output = input * 0.0001875;
                break;
        case GAIN_ONE: output = input * 0.000125;
                break;
        case GAIN_TWO: output = input * 0.0000625;
                break;
        case GAIN_FOUR: output = input * 0.00003125; 
                break; 
        case GAIN_EIGHT: output = input * 0.000015625; 
                 break; 
        case GAIN_SIXTEEN: output = input * 0.0000078125; 
                break; 
        default: debugE("FATAL: invalid GAIN parameter.");  
      } 
    }
    else {
      debugE("FATAL: chip parameter must be ADS1015chip or ADS1115chip.");
    }
    notify();
}
