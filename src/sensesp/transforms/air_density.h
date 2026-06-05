#ifndef SENSESP_TRANSFORMS_AIR_DENSITY_H_
#define SENSESP_TRANSFORMS_AIR_DENSITY_H_

#include <tuple>

#include "transform.h"

namespace sensesp {

/**
 * @brief Transforms temperature, relative humidity and pressure in air density.
 *
 * More info about the equation can be found at
 * https://en.wikipedia.org/wiki/Density_of_air
 */
class AirDensity : public Transform<std::tuple<float, float, float>, float> {
 public:
  AirDensity();
  virtual void set(const std::tuple<float, float, float>& input) override;
};

}  // namespace sensesp

#endif
