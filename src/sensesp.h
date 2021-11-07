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

namespace sensesp {

typedef std::function<void()> void_cb_func;

#ifndef DEBUG_DISABLED
extern DEBUG_CLASS Debug;
#endif

}  // namespace sensesp

#endif
