#include "signalk_listener.h"

namespace sensesp {

std::vector<SKListener*> SKListener::listeners;

SKListener::SKListener(String sk_path, int listen_delay)
    : sk_path{sk_path}, listen_delay{listen_delay} {
  listeners.push_back(this);
}

}  // namespace sensesp
