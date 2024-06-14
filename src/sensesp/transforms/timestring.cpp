
#include "timestring.h"

namespace sensesp {

TimeString::TimeString(String config_path)
    : Transform<time_t, String>(config_path) {}

void TimeString::set(time_t input) {
  char buf[sizeof "2011-10-08T07:07:09Z"];
  strftime(buf, sizeof buf, "%FT%TZ", gmtime(&input));
  this->emit(String(buf));
}

}  // namespace sensesp
