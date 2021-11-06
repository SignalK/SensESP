#ifndef _uuid_H_
#define _uuid_H_

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
