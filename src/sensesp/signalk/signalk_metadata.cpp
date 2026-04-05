#include "signalk_metadata.h"

namespace sensesp {

SKMetadata::SKMetadata(const String& units, const String& display_name,
                       const String& description, const String& short_name,
                       float timeout, bool supports_put)
    : display_name_{display_name},
      units_{units},
      description_{description},
      short_name_{short_name},
      timeout_{timeout},
      supports_put_{supports_put} {}

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

  if (this->supports_put_) {
    val["supportsPut"] = this->supports_put_;
  }
}

}  // namespace sensesp
