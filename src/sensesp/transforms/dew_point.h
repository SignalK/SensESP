#ifndef SENSESP_TRANSFORMS_DEW_POINT_H_
#define SENSESP_TRANSFORMS_DEW_POINT_H_

#include "transform.h"

namespace sensesp {

/**
 * @brief Transforms temperature and relative humidity in dew point temperature.
 *
 * This transform uses the Arden Buck equation with a maximum error of 0.06% for
 * temperatures from -40°C to +50°C. More info about the equation can be found
 * at https://en.wikipedia.org/wiki/Dew_point
 */
class DewPoint : public FloatTransform {
 public:
  DewPoint();
  virtual void set(const float& input) override;

 private:
  float inputs[2]{};
};

}  // namespace sensesp

#endif
