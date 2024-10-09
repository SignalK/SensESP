#include "sensesp.h"

#include "filesystem.h"

#include "SPIFFS.h"

namespace sensesp {

Filesystem::Filesystem() : Resettable(-100) {
  if (!SPIFFS.begin(true)) {
    ESP_LOGE(__FILENAME__, "FATAL: Filesystem initialization failed.");
    ESP.restart();
  }
  ESP_LOGI(__FILENAME__, "Filesystem initialized");
}

Filesystem::~Filesystem() {
  SPIFFS.end();
}

void Filesystem::reset() {
  ESP_LOGI(__FILENAME__, "Formatting filesystem");
  SPIFFS.format();
}

}  // namespace sensesp
