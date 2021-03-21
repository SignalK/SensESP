#include "heat_index.h"

// heat index temperature

HeatIndexTemperature::HeatIndexTemperature() : NumericTransform() {}

void HeatIndexTemperature::set_input(float input, uint8_t inputChannel) {
  inputs[inputChannel] = input;
  received |= 1 << inputChannel;
  if (received ==
      0b11) {  // for 2 channels, use 0b11. For 3 channels, use b111 and so on.
    received = 0;  // recalculates after all values are updated. Remove if a
                   // recalculation is required after an update of any value.

    // The following equation approximate the heat index.
    // For more info on the calculation see
    // https://en.wikipedia.org/wiki/Dew_point#Calculating_the_dew_point

    float temp_celsius =
        inputs[0] - 273.15;  // dry-bulb temperature (in degrees Celsius)
    float relative_humidity = inputs[1];

    // Coefficients for temperature in Celsius
    // Error is 0,7% for temperatures above 27°C and RH above 40%
    const double c1 = -8.78469475556;
    const double c2 = 1.61139411;
    const double c3 = 2.33854883889;
    const double c4 = -0.14611605;
    const double c5 = -0.012308094;
    const double c6 = -0.0164248277778;
    const double c7 = 0.002211732;
    const double c8 = 0.00072546;
    const double c9 = -0.000003582;

    // equation for heat index
    float heat_index_temperature =
        c1 + 
        c2 * temp_celsius + 
        c3 * relative_humidity +
        c4 * temp_celsius * relative_humidity + c5 * pow(temp_celsius, 2) +
        c6 * pow(relative_humidity, 2) +
        c7 * pow(temp_celsius, 2) * relative_humidity +
        c8 * temp_celsius * pow(relative_humidity, 2) +
        c9 * pow(temp_celsius, 2) * pow(relative_humidity, 2);

    this->emit(heat_index_temperature + 273.15);  // Kelvin is Celsius + 273.15
  }
}

// heat index effect (warning levels)

HeatIndexEffect::HeatIndexEffect() : Transform<float, String>() {}

void HeatIndexEffect::set_input(float input, uint8_t inputChannel) {
  float heat_index_temperature = input;
  String heat_index_effect = "";
  if (heat_index_temperature > 273.15 + 54) {
    heat_index_effect = "Extreme danger";
  } else if (heat_index_temperature > 273.15 + 41) {
    heat_index_effect = "Danger";
  } else if (heat_index_temperature > 273.15 + 32) {
    heat_index_effect = "Extreme Caution";
  } else if (heat_index_temperature > 273.15 + 27) {
    heat_index_effect + "Caution";
  }

  this->emit(heat_index_effect);
}
