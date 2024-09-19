#ifndef SENSP_SRC_SENSESP_SIGNALK_SIGNALK_TIME_H_
#define SENSP_SRC_SENSESP_SIGNALK_SIGNALK_TIME_H_

#include "sensesp/transforms/timestring.h"
#include "signalk_emitter.h"

namespace sensesp {

class SKOutputTime : public TimeString, public SKEmitter {
 public:
  SKOutputTime(const String& sk_path, const String& config_path = "");
  virtual void as_signalk_json(JsonDocument& doc) override;
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
};

}  // namespace sensesp

#endif
