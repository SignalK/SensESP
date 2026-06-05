#ifndef SENSESP_TRANSFORMS_HEAT_INDEX_H_
#define SENSESP_TRANSFORMS_HEAT_INDEX_H_

#include <tuple>

#include "transform.h"

namespace sensesp {

/**
 * @brief Transforms temperature and relative humidity in heat index
 * temperature.
 *
 * This transform uses an equation to approximate the heat index table from
 * the NOAA NWS. More info about the equation can be found at
 * https://en.wikipedia.org/wiki/Heat_index
 */
class HeatIndexTemperature : public Transform<std::tuple<float, float>, float> {
 public:
  HeatIndexTemperature();
  virtual void set(const std::tuple<float, float>& input) override;
};

/**
 * @brief Transforms heat index temperature to heat index effect.
 *
 * There are five effects: ""(no effect), "Caution", "Extreme caution, "Danger",
 * "Extreme danger". More info about the equation can be found at
 * https://en.wikipedia.org/wiki/Heat_index
 */
class HeatIndexEffect : public Transform<float, String> {
 public:
  HeatIndexEffect();
  virtual void set(const float& input) override;
};

}  // namespace sensesp

#endif
