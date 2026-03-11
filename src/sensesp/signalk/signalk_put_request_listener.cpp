#include "signalk_put_request_listener.h"

namespace sensesp {

//std::vector<SKPutListener*> SKPutListener::listeners_;

SKPutListener::SKPutListener(const String& sk_path) : sk_path{sk_path} {
  
  SKListener::listeners_.push_back(this);
  debugI("PutListener(%s)",sk_path.c_str());
   debugI("===========   SKListener::listeners_.size()=%d    ===========",SKListener::listeners_.size());
}

}  // namespace sensesp
