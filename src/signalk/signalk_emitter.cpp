#include "signalk_emitter.h"

std::vector<SKEmitter*> SKEmitter::sources;

SKEmitter::SKEmitter(String sk_path) : sk_path{sk_path} {
  sources.push_back(this);
}

void SKEmitter::add_metadata(JsonArray& meta) {
  SKMetadata* my_meta = this->get_metadata();
  if (my_meta != NULL) {
    my_meta->add_entry(this->get_sk_path(), meta);
  }
}
