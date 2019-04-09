#include "sensesp.h"

#include "spiffs_storage.h"

#include "FS.h"

void setup_spiffs_storage() {
  bool result = SPIFFS.begin();
  if (!result) {
    debugE("FATAL: Filesystem initialization failed.");
    ESP.restart();
  }
}

void write_string(const String filename, String data) {
  File f = SPIFFS.open(filename, "w");
  if (f) {
    f.print(data);
    f.close();
  }
}

String read_string(const String filename) {
  File f = SPIFFS.open(filename, "r");
  if (!f) {
    return "";
  }
  String input = f.readString();
  f.close();
  return input;
}
