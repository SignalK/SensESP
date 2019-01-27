#ifndef _fs_H_
#define _fs_H_

#include "Arduino.h"
#include "ArduinoJson.h"
#include "FS.h"

// quick and dirty hack to get string values to
// disk and back

void setup_spiffs_storage();

void write_string(const String filename, String data);
String read_string(const String filename);

void write_json(const String filename, JsonObject& data);
JsonObject& read_json(const String filename, JsonBuffer& buf);

#endif