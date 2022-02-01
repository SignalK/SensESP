#include "SPIFFS.h"

#include "filesystem.h"
#include "sensesp.h"

namespace sensesp {

Filesystem::Filesystem() : Resettable(-100) {
  if (!SPIFFS.begin(true)) {
    debugE("FATAL: Filesystem initialization failed.");
    ESP.restart();
  }
}

void Filesystem::reset() {
  debugI("Formatting filesystem");
  SPIFFS.format();
}

}  // namespace sensesp
