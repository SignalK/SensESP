#ifndef _signalk_delta_H_
#define _signalk_delta_H_

#include <list>

#include "ArduinoJson.h"

///////////////////
// Signal K delta message representation

class SKDelta {
 public:
  SKDelta(const String& hostname, unsigned int max_buffer_size=20);
  void append(const String val);
  bool data_available();
  void get_delta(String& output);
  void set_hostname(String hostname) { this->hostname = hostname; }
 private:
  String hostname;
  unsigned int max_buffer_size;
  std::list<String> buffer;
};

#endif
