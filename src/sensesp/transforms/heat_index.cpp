#include "heat_index.h"

namespace sensesp {

// heat index temperature

HeatIndexTemperature::HeatIndexTemperature() : FloatTransform() {}

void HeatIndexTemperature::set(const float& /*input*/) {
  // The following equation approximate the heat index
  // using both Steadman's and Rothfusz equations
  // See https://www.ncbi.nlm.nih.gov/pmc/articles/PMC3801457/
  // Algorithm 1 (table 1) (algorithm in figure 3)

  float const temp_fahrenheit =
      1.8 * (inputs[0] - 273.15) +
      32;  // dry-bulb temperature in degrees fahrenheit
  float const relative_humidity =
      inputs[1] * 100;  // relative humidity in percent

  // step 1: if temperature is less than 40°F then heat index temperature is
  // dry bulb temperature.
  float heat_index_temperature;

  if (temp_fahrenheit <= 40) {
    heat_index_temperature = temp_fahrenheit;
  } else {
    // step 2: use algorithm to calculate heat index and us it if the result
    // is less than 79°F;
    heat_index_temperature =
        -10.3 + 1.1 * temp_fahrenheit + 0.047 * relative_humidity;
    if (heat_index_temperature > 79) {
      // step 3: calculate heat index temperature for other temperatures
      // Coefficients for temperature in Fahrenheit
      const float c1 = -42.379;
      const float c2 = 2.04901523;
      const float c3 = 10.14333127;
      const float c4 = -0.22475541;
      const float c5 = -0.00683783;
      const float c6 = -0.05481717;
      const float c7 = 0.00122874;
      const float c8 = 0.00085282;
      const float c9 = -0.00000199;

      // equation for heat index
      float heat_index_temperature =
          c1 + c2 * temp_fahrenheit + c3 * relative_humidity +
          c4 * temp_fahrenheit * relative_humidity +
          c5 * pow(temp_fahrenheit, 2) + c6 * pow(relative_humidity, 2) +
          c7 * pow(temp_fahrenheit, 2) * relative_humidity +
          c8 * temp_fahrenheit * pow(relative_humidity, 2) +
          c9 * pow(temp_fahrenheit, 2) * pow(relative_humidity, 2);

      // step 4: if humidity =< 13% and temp between 80°F and 112°F then
      // correct heat index temperature
      if (relative_humidity <= 13 && 80 <= temp_fahrenheit &&
          temp_fahrenheit <= 112) {
        heat_index_temperature = heat_index_temperature -
                                 ((13 - relative_humidity) / 4) *
                                     sqrt(17 - abs(temp_fahrenheit - 95) / 17);
      }

      // step 5: if humidity => 85%  and temp between 80°C and 87°C then
      // correct heat index temperature
      else if (relative_humidity > 85 && 80 <= temp_fahrenheit &&
               temp_fahrenheit <= 87) {
        heat_index_temperature =
            heat_index_temperature +
            0.02 * (relative_humidity - 85) * (87 - temp_fahrenheit);
      }
    }

    this->emit((heat_index_temperature - 32) / 1.8 +
               273.15);  // Fahrenheit to Kelvin
  }
}

// heat index effect (warning levels)

HeatIndexEffect::HeatIndexEffect() : Transform<float, String>() {}

void HeatIndexEffect::set(const float& input) {
  const float heat_index_temperature =
      input - 273.15;  // celsius = kelvin - 273.15
  String heat_index_effect = "";
  if (heat_index_temperature > 54) {
    heat_index_effect = "Extreme danger";
  } else if (heat_index_temperature > 41) {
    heat_index_effect = "Danger";
  } else if (heat_index_temperature > 32) {
    heat_index_effect = "Extreme Caution";
  } else if (heat_index_temperature > 27) {
    heat_index_effect + "Caution";
  }

  this->emit(heat_index_effect);
}

}  // namespace sensesp
