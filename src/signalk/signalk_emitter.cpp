#include "signalk_emitter.h"

std::vector<SKEmitter*> SKEmitter::sources;

SKEmitter::Metadata::Metadata(String display_name, String units, String description, String short_name, String timeout) :
   display_name_{display_name},
   units_{units},
   description_{description},
   short_name_{short_name},
   timeout_{timeout} {
}

SKEmitter::SKEmitter(String sk_path) : sk_path{sk_path} {
  sources.push_back(this);
}

void SKEmitter::add_metadata(JsonArray& meta) {

   Metadata* my_meta = this->get_metadata();
   if (my_meta != NULL) {

      JsonObject json = meta.createNestedObject();
      json["path"] = this->get_sk_path();
      JsonObject val = json.createNestedObject("value");

      if (!my_meta->display_name_.isEmpty()) {
        val["displayName"] = my_meta->display_name_;
      }

      if (!my_meta->units_.isEmpty()) {
        val["units"] = my_meta->units_;
      }

      if (!my_meta->description_.isEmpty()) {
        val["description"] = my_meta->description_;
      }

      if (!my_meta->short_name_.isEmpty()) {
        val["shortName"] = my_meta->short_name_;
      }

      if (!my_meta->timeout_.isEmpty()) {
        val["timeout"] = my_meta->timeout_;
      }
   }
}