#include "sensesp.h"

#include "sensesp_base_app.h"

namespace sensesp {

reactesp::EventLoop* event_loop() {
  return SensESPBaseApp::get_event_loop();
}

}  // namespace sensesp
