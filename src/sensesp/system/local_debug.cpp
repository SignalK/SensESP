#ifndef DEBUG_DISABLED

#include "local_debug.h"

namespace sensesp {

bool LocalDebug::begin(String hostname, uint8_t startingDebugLevel) {
  _lastDebugLevel = startingDebugLevel;
}

boolean LocalDebug::isActive(uint8_t debugLevel) {
  return (debugLevel >= _lastDebugLevel);
}

// size_t LocalDebug::write(uint8_t character) {
//   Serial.write(character);
//   return 1;
// }

#else  // DEBUG_DISABLED

// define empty debug macros

#include "local_debug.h"

#endif  // DEBUG_DISABLED

}  // namespace sensesp
