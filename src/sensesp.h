#ifndef _sensesp_H_
#define _sensesp_H_

#include <ReactESP.h>

#ifdef REMOTE_DEBUG
#include <RemoteDebug.h>
#ifndef DEBUG_DISABLED
#define DEBUG_CLASS RemoteDebug
#endif  // DEBUG_DISABLED
#else  // REMOTE_DEBUG
#include "system/local_debug.h"
#ifndef DEBUG_DISABLED
#define DEBUG_CLASS LocalDebug
#endif  // DEBUG_DISABLED
#endif  // REMOTE_DEBUG

#if ESP8266
#error "ESP8266 isn't supported by SensESP version 2. To compile an existing project, add @^1.0.0 to the SensESP depdenency in platformio.ini."
#endif

namespace sensesp {

// Typically, pulling a namespace into another is something to be avoided,
// but ReactESP and SensESP are tightly knit together, and having to explicitly
// manage the reactesp namespace might be confusing for some users.
using namespace reactesp;

typedef std::function<void()> void_cb_func;

#ifndef DEBUG_DISABLED
extern DEBUG_CLASS Debug;
#endif

}  // namespace sensesp

#endif
