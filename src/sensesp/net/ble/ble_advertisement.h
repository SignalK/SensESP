#ifndef SENSESP_NET_BLE_BLE_ADVERTISEMENT_H_
#define SENSESP_NET_BLE_BLE_ADVERTISEMENT_H_

#include <Arduino.h>

#include <cstdint>
#include <vector>

namespace sensesp {

/**
 * @brief A single BLE advertisement received by the scanner.
 *
 * Plain value type with no Bluedroid / NimBLE dependencies so it can
 * be the public data type that BLE provisioner implementations emit
 * regardless of which BT host stack they use internally. Consumers
 * (for example, BLESignalKGateway) operate on this struct rather than
 * on stack-specific result types.
 *
 * All fields are populated at the moment the host stack hands the
 * advertisement to SensESP. Raw adv data is copied — the scanner does
 * not hold onto stack-internal buffers past the end of its callback.
 */
struct BLEAdvertisement {
  /// Peer address, formatted "AA:BB:CC:DD:EE:FF" (uppercase, colon-separated).
  String address;

  /// Peer address type (LE public, LE random, …). Encoded as the
  /// integer value the underlying stack returns; 0 = public, 1 =
  /// random static, 2 = random resolvable, 3 = random non-resolvable
  /// in the BT-SIG scheme.
  uint8_t address_type = 0;

  /// RSSI in dBm at the moment of reception.
  int8_t rssi = 0;

  /// Complete local name AD field, if present; otherwise empty.
  String name;

  /// Raw advertising payload bytes exactly as received, including
  /// all AD structures (length, type, value …). Useful for generic
  /// consumers that want to parse fields the scanner did not
  /// pre-extract.
  std::vector<uint8_t> adv_data;

  /// Raw scan-response bytes, if the peer sent any (only populated
  /// on active scans where the peer supported scan responses).
  std::vector<uint8_t> scan_rsp_data;

  /// Wall-clock time of reception from millis() on the host. Used
  /// by consumers that need to age out stale entries or order by
  /// arrival time.
  unsigned long received_at_ms = 0;
};

}  // namespace sensesp

#endif  // SENSESP_NET_BLE_BLE_ADVERTISEMENT_H_
