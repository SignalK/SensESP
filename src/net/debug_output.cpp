#include "net/debug_output.h"

#ifdef REMOTE_DEBUG
#include <RemoteDebug.h>
#else
#include "system/local_debug.h"
#endif

#include "sensesp_app.h"

namespace sensesp {

#ifndef DEBUG_DISABLED
DEBUG_CLASS Debug;
#endif

void DebugOutput::start() {
#ifndef DEBUG_DISABLED
  Debug.begin(SensESPBaseApp::get_hostname());
  Debug.setResetCmdEnabled(true);
// serial port debugging happens synchronously
#ifdef REMOTE_DEBUG
  // serial port debugging happens synchronously
  ReactESP::app->onRepeat(1, []() { Debug.handle(); });
#endif
#endif
}

}  // namespace sensesp
