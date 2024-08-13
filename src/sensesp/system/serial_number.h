#ifndef SENSESP_SRC_SENSESP_SYSTEM_SERIAL_NUMBER_H_
#define SENSESP_SRC_SENSESP_SYSTEM_SERIAL_NUMBER_H_

#include <cstdint>
#include <esp_mac.h>

uint64_t GetBoardSerialNumber() {
  uint8_t chipid[6];
  esp_efuse_mac_get_default(chipid);
  return ((uint64_t)chipid[0] << 0) + ((uint64_t)chipid[1] << 8) +
         ((uint64_t)chipid[2] << 16) + ((uint64_t)chipid[3] << 24) +
         ((uint64_t)chipid[4] << 32) + ((uint64_t)chipid[5] << 40);
}

#endif  // SENSESP_SRC_SENSESP_SYSTEM_SERIAL_NUMBER_H_
