#ifndef _timestring_H_
#define _timestring_H_

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
  TimeString(String config_path = "");
  virtual void set_input(time_t input, uint8_t input_channel = 0) override;
};

}  // namespace sensesp
#endif
