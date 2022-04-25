#include "local_debug.h"

namespace sensesp {

#ifndef DEBUG_DISABLED

bool LocalDebug::begin(String hostname, uint8_t startingDebugLevel) {
  lastDebugLevel_ = startingDebugLevel;
  return true;
}

boolean LocalDebug::isActive(uint8_t debugLevel) {
  return (debugLevel >= lastDebugLevel_);
}

// size_t LocalDebug::write(uint8_t character) {
//   Serial.write(character);
//   return 1;
// }

#endif  // DEBUG_DISABLED

}  // namespace sensesp
