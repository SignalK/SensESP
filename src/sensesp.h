#ifndef _SENSESP_H_
#define _SENSESP_H_

#include <ReactESP.h>

#include "sensesp/system/local_debug.h"

#if ESP8266
#error \
    "ESP8266 isn't supported by SensESP version 2. To compile an existing project, add @^1.0.0 to the SensESP depdenency in platformio.ini."
#endif

#ifndef CORE_DEBUG_LEVEL
#error "SensESP v3 requires CORE_DEBUG_LEVEL to be defined. See https://signalk.org/SensESP/pages/migration/."
#endif

namespace sensesp {

// Typically, pulling a namespace into another is something to be avoided,
// but ReactESP and SensESP are tightly knit together, and having to explicitly
// manage the reactesp namespace might be confusing for some users.
using namespace reactesp;

typedef std::function<void()> void_cb_func;

}  // namespace sensesp

#endif
