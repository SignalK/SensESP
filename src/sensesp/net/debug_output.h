#ifndef _remote_debug_H_
#define _remote_debug_H_

#include <cstdint>

namespace sensesp {

void SetupSerialDebug(uint32_t baudrate);

/**
 * @brief Initialize logging/debug output.
 *
 * The actual output mechanism is controlled by preprocessor macros defined
 * in platformio.ini.
 *
 * If DEBUG_DISABLED is defined, all output is disabled.
 * If REMOTE_DEBUG is defined, output is enabled both over the serial interface
 * and over telnet on port 23.
 * By default, output is enabled only on the serial interface.
 **/
class DebugOutput {
 public:
  DebugOutput();
};

}  // namespace sensesp

#endif
