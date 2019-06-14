#ifndef _signalk_time_H_
#define _signalk_time_H_

#include "transforms/timestring.h"
#include "signalk_emitter.h"

class SKOutputTime : public TimeString, 
                    public SKEmitter {
 public:
  SKOutputTime(String sk_path, String config_path="");
  virtual String as_signalK() override;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
};

#endif
