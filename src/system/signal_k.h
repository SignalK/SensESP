#ifndef _signal_k_H_
#define _signal_k_H_

#include <list>

#include "ArduinoJson.h"

///////////////////
// Signal K delta message representation

class SKDelta {
 public:
  SKDelta(const String& hostname, int max_buffer_size=20);
  void append(const String val);
  bool data_available();
  void get_delta(String& output);
 private:
  const String& hostname;
  int max_buffer_size;
  std::list<String> buffer;
};

#endif
