#ifndef SENSESP_NET_BLE_GATT_SESSION_H_
#define SENSESP_NET_BLE_GATT_SESSION_H_

#include <Arduino.h>

#include <cstdint>
#include <vector>

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

namespace sensesp {

enum class GATTSessionState {
  kConnecting,
  kInitializing,   // Running init writes sequentially
  kSubscribing,    // Registering for notifications sequentially
  kActive,         // Fully set up, forwarding data
  kDisconnecting,
  kClosed
};

struct InitWrite {
  String char_uuid;
  std::vector<uint8_t> data;
};

struct PollDescriptor {
  String char_uuid;
  uint32_t interval_ms;
};

struct PeriodicWriteDescriptor {
  String char_uuid;
  std::vector<uint8_t> data;
  uint32_t interval_ms;
};

/// Per-GATT-session state managed by BLESignalKGateway.
struct GATTSession {
  String session_id;
  String mac;
  uint8_t addr_type = 0;
  String service_uuid;
  int conn_handle = -1;

  GATTSessionState state = GATTSessionState::kConnecting;

  // Descriptors from the gatt_subscribe command
  std::vector<String> notify_uuids;
  std::vector<InitWrite> init_writes;
  std::vector<PollDescriptor> polls;
  std::vector<PeriodicWriteDescriptor> periodic_writes;

  // Progress through sequential init/subscribe steps
  size_t init_write_index = 0;
  size_t subscribe_index = 0;

  // Active FreeRTOS timers (cleaned up on close)
  std::vector<TimerHandle_t> timers;

  void delete_timers() {
    for (auto t : timers) {
      xTimerStop(t, 0);
      xTimerDelete(t, 0);
    }
    timers.clear();
  }
};

}  // namespace sensesp

#endif  // SENSESP_NET_BLE_GATT_SESSION_H_
