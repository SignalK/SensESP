#include "uuid.h"

namespace sensesp {

String generate_uuid4() {
  // implementation copied from https://github.com/protohaus/ESP_UUID
  // to avoid additional external dependencies.
  uint8_t buffer_[16];
  String uuid_str;

  // Generate 16 random bytes. On ESP32, these are actually random if the
  // radio is enabled.
  for (int i = 0; i < 16; i += 4) {
    uint32_t random = esp_random();
    memcpy(&buffer_[i], &random, 4);
  }

  buffer_[6] = (buffer_[6] & 0x0F) | 0x40;  // version 4
  buffer_[8] = (buffer_[8] & 0x3F) | 0x80;  // variant 1

  uuid_str.reserve(36 + 1);  // Include NULL / terminator byte

  for (int i = 0; i < 16; i++) {
    if (i == 4 || i == 6 || i == 8 || i == 10) {
      uuid_str += "-";
    }
    uuid_str += String(buffer_[i] >> 4, HEX);
    uuid_str += String(buffer_[i] & 0x0F, HEX);
  }
  return uuid_str;
}

}  // namespace sensesp
