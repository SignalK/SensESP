#ifndef SENSESP_SYSTEM_HASH_H
#define SENSESP_SYSTEM_HASH_H

#include "sensesp.h"

void Sha1(String payload_str, uint8_t *hash_output);
String MD5(String payload_str);

String Base64Sha1(String payload_str);

#endif
