#include "SPIFFS.h"
#define SPIFFS_BEGIN SPIFFS.begin(true)

#include "filesystem.h"
#include "sensesp.h"

namespace sensesp {

Filesystem::Filesystem() : Resettable(-100) {
  if (!SPIFFS_BEGIN) {
    debugE("FATAL: Filesystem initialization failed.");
    ESP.restart();
  }
}

void Filesystem::reset() {
  debugI("Formatting filesystem");
  SPIFFS.format();
}

}  // namespace sensesp
