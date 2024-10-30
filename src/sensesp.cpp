#include "sensesp.h"

#include <memory>

#include "sensesp_base_app.h"

namespace sensesp {

std::shared_ptr<reactesp::EventLoop> event_loop() {
  return SensESPBaseApp::get_event_loop();
}

}  // namespace sensesp
