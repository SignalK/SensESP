#ifndef SENSESP_SYSTEM_HASH_H
#define SENSESP_SYSTEM_HASH_H

#include "sensesp.h"

namespace sensesp {

void Sha1(const String& payload_str, uint8_t *hash_output);
String MD5(const String& payload_str);

String Base64Sha1(const String& payload_str);

}  // namespace sensesp

#endif
