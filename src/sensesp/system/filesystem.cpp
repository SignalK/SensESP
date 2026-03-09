#include "sensesp.h"

#include "filesystem.h"

#include "SPIFFS.h"

namespace sensesp {


Filesystem::Filesystem() : Resettable(-100) {
  // WOKWI-PATCH: Wir geben explizit den Partitionsnamen "vfs" an.
  // Der Parameter 'true' sorgt für die Formatierung, falls sie leer ist.
  if (!SPIFFS.begin(true, "/spiffs", 10, "vfs")) {
    // Hier ist der entscheidende Unterschied: Kein ESP.restart() mehr!
    ESP_LOGW(__FILENAME__, "WOKWI-PATCH: Partition 'vfs' nicht gefunden oder Mount fehlgeschlagen.");
    ESP_LOGW(__FILENAME__, "App laeuft ohne Filesystem weiter (RAM-Modus).");
  } else {
    ESP_LOGI(__FILENAME__, "Filesystem auf Partition 'vfs' erfolgreich initialisiert");
  }
}
Filesystem::~Filesystem() {
  SPIFFS.end();
}

void Filesystem::reset() {
  ESP_LOGI(__FILENAME__, "Formatting filesystem partition 'vfs'");
  // Auch beim Reset stellen wir sicher, dass wir die richtige Partition meinen
  SPIFFS.format(); 
}

}  // namespace sensesp
