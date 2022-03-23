#include "local_debug.h"

namespace sensesp {

#ifndef DEBUG_DISABLED

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

#endif  // DEBUG_DISABLED

}  // namespace sensesp
