#ifndef _heat_index_H_
#define _heat_index_H_

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
class HeatIndexTemperature : public FloatTransform {
 public:
  HeatIndexTemperature();
  virtual void set(float input, uint8_t inputChannel) override;

 private:
  uint8_t received = 0;
  float inputs[2];
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
  virtual void set(float input, uint8_t inputChannel = 0) override;
};

}  // namespace sensesp

#endif
