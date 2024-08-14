#include "dew_point.h"

namespace sensesp {

// dew point

DewPoint::DewPoint() : FloatTransform() {}

void DewPoint::set(const float& input) {
  // Dew point is calculated with Arden Buck Equation and Arden Buck valuation
  // sets For more info on the calculation see
  // https://en.wikipedia.org/wiki/Dew_point#Calculating_the_dew_point

  float temp_celsius = inputs[0] - 273.15;
  float relative_humidity = inputs[1];

  // valuation set for temperatures above 0°C
  float b = 17.368;
  float c = 238.88;
  const float d = 234.5;

  // valuation set for temperatures below 0°C
  if (temp_celsius < 0.0) {
    b = 17.966;
    c = 247.15;
  }

  float gamma =
      log(relative_humidity *
          exp((b - (temp_celsius / d)) * (temp_celsius / (c + temp_celsius))));
  float dew_point = (c * gamma) / (b - gamma);

  this->emit(dew_point + 273.15);  // Kelvin is Celsius + 273.15
}

}  // namespace sensesp
