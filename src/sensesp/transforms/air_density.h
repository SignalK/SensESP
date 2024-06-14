#ifndef _air_density_H_
#define _air_density_H_

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
  virtual void set(float input) override;

 private:
  float inputs[3];
};

}  // namespace sensesp

#endif
