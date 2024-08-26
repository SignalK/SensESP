#include "air_density.h"

#include <cmath>

namespace sensesp {

// dew point

AirDensity::AirDensity() : FloatTransform() {}

void AirDensity::set(const float& /*input*/) {
  // For more info on the calculation see
  // https://en.wikipedia.org/wiki/Density_of_air

  const float temp_kelvin = inputs[0];
  const float temp_celsius = temp_kelvin - 273.15;
  const float relative_humidity = inputs[1];
  const float pressure = inputs[2];

  // Saturation vapor pressure of water
  float saturation_pressure =
      6.1078 * pow(10, ((7.5 * temp_celsius) / (temp_celsius + 237.3)));

  // Vapor pressure of water
  const float vapor_pressure_water = relative_humidity * saturation_pressure;

  // Partial pressure of dry air
  const float partial_pressure_dry_air = pressure - vapor_pressure_water;

  // air density of humid air
  // const float specific_gas_constant_dry_air = 287.058;
  // const float specific_gas_constant_water_vapor = 461.495;
  const float molar_mass_dry_air = 0.0289652;
  const float molar_mass_water_vapor = 0.01801;
  const float universal_gas_constant = 8.31446;
  float air_density_humid_air =
      ((partial_pressure_dry_air * molar_mass_dry_air) +
       (vapor_pressure_water * molar_mass_water_vapor)) /
      (universal_gas_constant * temp_kelvin);

  this->emit(air_density_humid_air);
}

}  // namespace sensesp
