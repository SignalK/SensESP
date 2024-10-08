#ifndef SENSP_SRC_SENSESP_SIGNALK_SIGNALK_TIME_H_
#define SENSP_SRC_SENSESP_SIGNALK_SIGNALK_TIME_H_

#include "sensesp/ui/config_item.h"
#include "sensesp/transforms/timestring.h"
#include "signalk_emitter.h"

namespace sensesp {

class SKOutputTime : public TimeString, public SKEmitter {
 public:
  SKOutputTime(const String& sk_path, const String& config_path = "");
  virtual void as_signalk_json(JsonDocument& doc) override;
  virtual bool to_json(JsonObject& doc) override;
  virtual bool from_json(const JsonObject& config) override;
};

const String ConfigSchema(const SKOutputTime& obj);

}  // namespace sensesp

#endif
