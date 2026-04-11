#ifndef SENSESP_NET_BLE_NATIVE_BLE_H_
#define SENSESP_NET_BLE_NATIVE_BLE_H_

// BLEProvisioner for chips with a native BT controller using Bluedroid.
// Uses IDF Bluedroid GAP API directly (async, non-blocking scan).

#include <Arduino.h>
#include "sdkconfig.h"

#if defined(CONFIG_BT_BLUEDROID_ENABLED) && \
    !defined(CONFIG_ESP_HOSTED_ENABLE_BT_BLUEDROID)

#include <atomic>

#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"

#include "sensesp/net/ble/ble_provisioner.h"
#include "sensesp/net/ble/bluedroid_gattc.h"

namespace sensesp {

class NativeBLE;

struct NativeBLEConfig {
  using ProvisionerType = NativeBLE;
  bool active_scan = true;
  uint32_t scan_interval_ms = 100;
  uint32_t scan_window_ms = 100;
};

class NativeBLE : public BLEProvisioner {
 public:
  explicit NativeBLE(const NativeBLEConfig& config = {});
  ~NativeBLE() override;

  NativeBLE(const NativeBLE&) = delete;
  NativeBLE& operator=(const NativeBLE&) = delete;

  bool start_scan() override;
  bool stop_scan() override;
  bool is_scanning() const override;
  String mac_address() const override;
  uint32_t scan_hit_count() const override { return scan_hit_count_; }

#ifdef CONFIG_BT_GATTC_ENABLE
  int max_gatt_connections() const override {
    return gattc_.max_gatt_connections();
  }
  int active_gatt_connections() const override {
    return gattc_.active_gatt_connections();
  }
  int gatt_connect(const String& mac, uint8_t addr_type,
                   const String& service_uuid,
                   GATTConnectionCallbacks callbacks) override {
    return gattc_.gatt_connect(mac, addr_type, service_uuid,
                               std::move(callbacks));
  }
  bool gatt_subscribe_notify(int conn_handle,
                             const String& char_uuid) override {
    return gattc_.gatt_subscribe_notify(conn_handle, char_uuid);
  }
  bool gatt_read(int conn_handle, const String& char_uuid) override {
    return gattc_.gatt_read(conn_handle, char_uuid);
  }
  bool gatt_write(int conn_handle, const String& char_uuid,
                  const uint8_t* data, size_t len) override {
    return gattc_.gatt_write(conn_handle, char_uuid, data, len);
  }
  void gatt_disconnect(int conn_handle) override {
    gattc_.gatt_disconnect(conn_handle);
  }
#endif

 private:
  void handle_gap_event(esp_gap_ble_cb_event_t event,
                        esp_ble_gap_cb_param_t* param);
  static void gap_event_trampoline(esp_gap_ble_cb_event_t event,
                                   esp_ble_gap_cb_param_t* param);
  static NativeBLE* instance_;

  NativeBLEConfig config_;
  std::atomic<bool> bt_stack_up_{false};
  std::atomic<bool> scanning_{false};
  std::atomic<uint32_t> scan_hit_count_{0};
  esp_ble_scan_params_t scan_params_{};

#ifdef CONFIG_BT_GATTC_ENABLE
  BluedroidGATTC gattc_;
  static void gattc_event_trampoline(esp_gattc_cb_event_t event,
                                     esp_gatt_if_t gattc_if,
                                     esp_ble_gattc_cb_param_t* param);
#endif
};

}  // namespace sensesp

#endif  // CONFIG_BT_BLUEDROID_ENABLED && !CONFIG_ESP_HOSTED_ENABLE_BT_BLUEDROID
#endif  // SENSESP_NET_BLE_NATIVE_BLE_H_
