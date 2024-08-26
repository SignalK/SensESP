#ifndef SENSESP_SYSTEM_UUID_H
#define SENSESP_SYSTEM_UUID_H

#include "Arduino.h"

namespace sensesp {

/**
 * @brief Generate a random UUIDv4 string.
 *
 * @return Formatted UUID string.
 */
String generate_uuid4();

}  // namespace sensesp

#endif
