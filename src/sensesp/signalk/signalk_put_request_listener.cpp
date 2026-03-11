#include "signalk_put_request_listener.h"

namespace sensesp {

std::vector<SKPutListener*> SKPutListener::listeners_;

SKPutListener::SKPutListener(const String& sk_path) : sk_path{sk_path} {
  
  SKPutListener::listeners_.push_back(this);
  debugI("SKPutListener(%s)",sk_path.c_str());
   debugI("===========   SKPutListener::listeners_.size()=%d    ===========",SKPutListener::listeners_.size());
}

}  // namespace sensesp
