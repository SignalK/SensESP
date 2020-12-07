#include "signalk_metadata.h"

SKMetadata::SKMetadata(String units, String display_name, String description,
                       String short_name, String timeout)
    : units_{units},
      display_name_{display_name},
      description_{description},
      short_name_{short_name},
      timeout_{timeout} {        
}


void SKMetadata::add_entry(String sk_path, JsonArray& meta) {
  JsonObject json = meta.createNestedObject();
  json["path"] = sk_path;
  JsonObject val = json.createNestedObject("value");

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

  if (!this->timeout_.isEmpty()) {
    val["timeout"] = this->timeout_;
  }
}
