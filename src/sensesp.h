#ifndef _sensesp_H_
#define _sensesp_H_

#include <ReactESP.h>
#include <RemoteDebug.h>

typedef std::function<void()> void_cb_func;

extern ReactESP app;

#ifndef DEBUG_DISABLED
extern RemoteDebug Debug;
#endif

#endif
