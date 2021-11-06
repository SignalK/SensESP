#include "signalk_put_request_listener.h"

namespace sensesp {

std::vector<SKPutListener*> SKPutListener::listeners;

SKPutListener::SKPutListener(String sk_path) : sk_path{sk_path} {
  listeners.push_back(this);
}

}  // namespace sensesp
