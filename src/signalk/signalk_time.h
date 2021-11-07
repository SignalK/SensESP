#ifndef _signalk_time_H_
#define _signalk_time_H_

#include "signalk_emitter.h"
#include "transforms/timestring.h"

namespace sensesp {

class SKOutputTime : public TimeString, public SKEmitter {
 public:
  SKOutputTime(String sk_path, String config_path = "");
  virtual String as_signalk() override;
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
};

}  // namespace sensesp

#endif
