#include "battery.h"

#include "sensesp.h"
#include <RemoteDebug.h>



// Battery represents a LifePo4 battery.
Battery::Battery (uint8_t ADSVoltAddr   )
{
   blp_ = new BLP(  ADSVoltAddress  );
    blp_->begin();
};


void BatteryValue::enable() {
  app.onRepeat(read_delay_, [this](){
      if (val_type_ == voltage) {
        output = battery_->blp_->voltage();
      } else if (val_type_ == overvoltage) {
        output = battery_->blp_->overVoltage();
      }      
      else {
        debugE("BatteryValue:enable(): Didn't recognize the val_type.");
      }

      notify();
  });
}

_VOID BatteryValue::get_configuration(JsonObject& root) {
  root["read_delay"] = read_delay_;
}

static const char SCHEMA[] PROGMEM = R"###({
  "type": "object",
  "properties": {
      "read_delay": { "title": "Read delay", "type": "number", "description": "The time, in milliseconds, between each read of the input" }
  }
})###";


String BatteryValue::get_config_schema() {
  return FPSTR(SCHEMA);
}

bool BatteryValue::set_configuration(const JsonObject& config) {
  String expected[] = {"read_delay"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  read_delay_ = config["read_delay"];
  return true;
}