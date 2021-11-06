#include "net/remote_debugger.h"

#include <RemoteDebug.h>

#include "sensesp_app.h"

namespace sensesp {

// initialize remote debugging

void RemoteDebugger::start() {
#ifndef DEBUG_DISABLED
  // Debug.begin(SensESPBaseApp::get()->get_hostname());
  // Debug.setResetCmdEnabled(true);
  // ReactESP::app->onRepeat(1, []() { Debug.handle(); });
#endif
}

}  // namespace sensesp
