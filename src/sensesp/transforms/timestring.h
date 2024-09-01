#ifndef SENSESP_TRANSFORMS_TIMESTRING_H_
#define SENSESP_TRANSFORMS_TIMESTRING_H_

#include <ctime>

#include "sensesp/system/valueconsumer.h"
#include "transform.h"

namespace sensesp {

/**
 * @brief Consumes a time_t time object and produces a human readable
 * string of the time for UTC in ISO 8601 format.
 */
class TimeString : public Transform<time_t, String> {
 public:
  TimeString(const String& config_path = "");
  virtual void set(const time_t& input) override;
};

}  // namespace sensesp
#endif
