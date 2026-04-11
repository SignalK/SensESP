#ifndef SENSESP_NET_BLE_NIMBLE_BLE_H_
#define SENSESP_NET_BLE_NIMBLE_BLE_H_

// BLEProvisioner for chips with a native BT controller using NimBLE.
// Lighter on RAM than Bluedroid (~15KB less internal SRAM), making it
// suitable for WiFi+BLE on memory-constrained chips like ESP32-C5.

#include <Arduino.h>
#include "sdkconfig.h"

#if defined(CONFIG_BT_NIMBLE_ENABLED) && \
    !defined(CONFIG_ESP_HOSTED_ENABLE_BT_NIMBLE)

#include <atomic>

#include "host/ble_gap.h"

#include "sensesp/net/ble/ble_provisioner.h"

namespace sensesp {

class NimBLEProvisioner;

struct NimBLEProvisionerConfig {
  using ProvisionerType = NimBLEProvisioner;
  bool active_scan = false;
  uint32_t scan_interval_ms = 320;
  uint32_t scan_window_ms = 160;
};

class NimBLEProvisioner : public BLEProvisioner {
 public:
  explicit NimBLEProvisioner(const NimBLEProvisionerConfig& config = {});
  ~NimBLEProvisioner() override;

  NimBLEProvisioner(const NimBLEProvisioner&) = delete;
  NimBLEProvisioner& operator=(const NimBLEProvisioner&) = delete;

  bool start_scan() override;
  bool stop_scan() override;
  bool is_scanning() const override;
  String mac_address() const override;
  uint32_t scan_hit_count() const override { return scan_hit_count_; }

 private:
  // NimBLE GAP event callback — static, dispatches to instance_.
  static int gap_event_handler(struct ble_gap_event* event, void* arg);
  static NimBLEProvisioner* instance_;

  NimBLEProvisionerConfig config_;
  std::atomic<bool> scanning_{false};
  std::atomic<uint32_t> scan_hit_count_{0};
};

}  // namespace sensesp

#endif  // CONFIG_BT_NIMBLE_ENABLED && !CONFIG_ESP_HOSTED_ENABLE_BT_NIMBLE
#endif  // SENSESP_NET_BLE_NIMBLE_BLE_H_
