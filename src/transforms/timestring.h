#ifndef _timestring_H_
#define _timestring_H_

#include <ctime>

#include "transform.h"

class TimeString : public Transform {
 public:
  TimeString(String sk_path, String id="", String schema="");
  String get() { return output; }
  void set_input(time_t input);
  String as_json() override final;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override final;
  virtual bool set_configuration(const JsonObject& config) override final;
 private:
  String output;
};

#endif
