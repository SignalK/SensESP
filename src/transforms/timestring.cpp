
#include "timestring.h"


TimeString::TimeString(String config_path) :
    Transform<time_t, String>(config_path) {
      className = "TimeString";
}

void TimeString::set_input(time_t input, uint8_t inputChannel) {
  char buf[sizeof "2011-10-08T07:07:09Z"];
  strftime(buf, sizeof buf, "%FT%TZ", gmtime(&input));
  output = String(buf);
  notify();
}
