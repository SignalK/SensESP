#include "signalk_metadata.h"

#include <cmath>

namespace sensesp {

SKMetadata::SKMetadata(const String& units, const String& display_name,
                       const String& description, const String& short_name,
                       float timeout)
    : display_name_{display_name},
      units_{units},
      description_{description},
      short_name_{short_name},
      timeout_{timeout},
      supports_put_{-1},
      display_scale_lower_{NAN},
      display_scale_upper_{NAN} {}

void SKMetadata::add_entry(const String& sk_path, JsonArray& meta) {
  JsonObject json = meta.add<JsonObject>();
  json["path"] = sk_path;
  JsonObject val = json["value"].to<JsonObject>();

  if (!this->display_name_.isEmpty()) {
    val["displayName"] = this->display_name_;
  }

  if (!this->units_.isEmpty()) {
    val["units"] = this->units_;
  }

  if (!this->description_.isEmpty()) {
    val["description"] = this->description_;
  }

  if (!this->short_name_.isEmpty()) {
    val["shortName"] = this->short_name_;
  }

  if (this->timeout_ >= 0.0) {
    val["timeout"] = this->timeout_;
  }

  if (!this->example_.isEmpty()) {
    val["example"] = this->example_;
  }

  if (this->supports_put_ != -1) {
    val["supportsPut"] = (this->supports_put_ == 1);
  }

  if (!std::isnan(this->display_scale_lower_) ||
      !std::isnan(this->display_scale_upper_)) {
    JsonObject scale = val["displayScale"].to<JsonObject>();
    if (!std::isnan(this->display_scale_lower_)) {
      scale["lower"] = this->display_scale_lower_;
    }
    if (!std::isnan(this->display_scale_upper_)) {
      scale["upper"] = this->display_scale_upper_;
    }
  }

  if (!this->zones_.empty()) {
    JsonArray zones_arr = val["zones"].to<JsonArray>();
    for (const SKZone& zone : this->zones_) {
      JsonObject zone_obj = zones_arr.add<JsonObject>();
      zone_obj["state"] = alarm_state_to_string(zone.state_);
      zone_obj["message"] = zone.message_;
      if (!std::isnan(zone.lower_)) zone_obj["lower"] = zone.lower_;
      if (!std::isnan(zone.upper_)) zone_obj["upper"] = zone.upper_;
    }
  }
}

const char* SKMetadata::alarm_state_to_string(SKAlarmState state) {
  switch (state) {
    case SKAlarmState::kNominal:   return "nominal";
    case SKAlarmState::kNormal:    return "normal";
    case SKAlarmState::kAlert:     return "alert";
    case SKAlarmState::kWarn:      return "warn";
    case SKAlarmState::kAlarm:     return "alarm";
    case SKAlarmState::kEmergency: return "emergency";
    default:                       return "normal";
  }
}

}  // namespace sensesp
