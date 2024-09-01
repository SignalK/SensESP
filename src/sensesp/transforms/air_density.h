#ifndef SENSESP_TRANSFORMS_AIR_DENSITY_H_
#define SENSESP_TRANSFORMS_AIR_DENSITY_H_

#include "transform.h"

namespace sensesp {

/**
 * @brief Transforms temperature, relative humidity and pressure in air density.
 *
 * More info about the equation can be found at
 * https://en.wikipedia.org/wiki/Density_of_air
 */
class AirDensity : public FloatTransform {
 public:
  AirDensity();
  virtual void set(const float& input) override;

 private:
  float inputs[3]{};
};

}  // namespace sensesp

#endif
