#ifndef _signal_k_H_
#define _signal_k_H_

#include <list>

#include "ArduinoJson.h"

///////////////////
// Signal K delta message representation

class SKDelta {
    String hostname;
    std::list<String> buffer;
  public:
    SKDelta(const String hostname);
    void append(const String val);
    bool data_available();
    void get_delta(String& output);
};

#endif
