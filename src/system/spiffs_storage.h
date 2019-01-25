#ifndef _fs_H_
#define _fs_H_

#include "FS.h"
#include "Arduino.h"

// quick and dirty hack to get string values to
// disk and back

void setup_spiffs_storage();

void write_string(const String filename, String data);
String read_string(const String filename);

#endif