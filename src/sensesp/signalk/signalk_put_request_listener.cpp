#include "signalk_put_request_listener.h"

namespace sensesp {

std::vector<SKPutListener*> SKPutListener::listeners_;

SKPutListener::SKPutListener(const String& sk_path) : sk_path{sk_path} {
  listeners_.push_back(this);
}

}  // namespace sensesp
