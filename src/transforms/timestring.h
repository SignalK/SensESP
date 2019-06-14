#ifndef _timestring_H_
#define _timestring_H_

#include <ctime>

#include "transform.h"
#include "system/valueconsumer.h"

/**
 * TimeString consumes a time_t time object and produces a human readable
 * string of the time for UTC in ISO 8601 format.
 */
class TimeString : public Transform<time_t, String> {
                          
 public:
  TimeString(String config_path="");
  virtual void set_input(time_t input, uint8_t inputChannel = 0) override;

};

#endif
