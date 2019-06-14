#include "signalk_emitter.h"

std::vector<SKEmitter*> SKEmitter::sources;

SKEmitter::SKEmitter(String sk_path) : sk_path{sk_path} {
  sources.push_back(this);
}
