#include "dew_point.h"

// dew point

DewPoint::DewPoint() : NumericTransform() {}

void DewPoint::set_input(float input, uint8_t inputChannel) {
  inputs[inputChannel] = input;
  received |= 1 << inputChannel;
  if (received == 0b11) {
    received = 0;

          float TC= inputs[0]-273.15;     // temperature in Celsius
          float RH= inputs[1];            // relative humidity

          // https://en.wikipedia.org/wiki/Dew_point#Calculating_the_dew_point, with Arden Buck Equation and Arden Buck valuation sets

          // valuation set for temperatures above 0°C
          float b = 17.368;
          float c = 238.88; 
          const float d = 234.5; 
          
          // valuation set for temperatures below 0°C
          if (TC<0.0) {
            b = 17.966;
            c = 247.15; 
          }

          float gamma=log(RH*exp((b-(TC/d))*(TC/(c+TC))));
          float dpt = (c*gamma)/(b-gamma);

  this->emit(dpt + 273.15); // Kelvin is Celsius + 273.15
  }
}