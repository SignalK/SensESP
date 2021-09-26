#ifdef ESP32
#include "SPIFFS.h"
#define SPIFFS_BEGIN SPIFFS.begin(true)
#elif defined(ESP8266)
#include "FS.h"
#define SPIFFS_BEGIN SPIFFS.begin()
#endif


#include "filesystem.h"
#include "sensesp.h"

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
