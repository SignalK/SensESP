#include <RemoteDebug.h>

#include "net/remote_debugger.h"
#include "sensesp_app.h"

// initialize remote debugging

void RemoteDebugger::start() {
#ifndef DEBUG_DISABLED
  //Debug.begin(sensesp_app->get_hostname());
  //Debug.setResetCmdEnabled(true);
  //app.onRepeat(1, []() { Debug.handle(); });
#endif
}


