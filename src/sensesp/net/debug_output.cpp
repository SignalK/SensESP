#include "debug_output.h"

#include "sensesp/system/local_debug.h"
#include "sensesp_app.h"

namespace sensesp {

#ifndef DEBUG_DISABLED
DEBUG_CLASS Debug;
#endif

DebugOutput::DebugOutput() {
#ifndef DEBUG_DISABLED
  Debug.begin();
  Debug.setResetCmdEnabled(true);
// serial port debugging happens synchronously
#ifdef REMOTE_DEBUG
  // serial port debugging happens synchronously
  ReactESP::app->onRepeat(1, []() { Debug.handle(); });
#endif
#endif
}

}  // namespace sensesp
