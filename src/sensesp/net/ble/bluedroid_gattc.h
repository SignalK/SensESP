#ifndef SENSESP_NET_BLE_BLUEDROID_GATTC_H_
#define SENSESP_NET_BLE_BLUEDROID_GATTC_H_

// Shared Bluedroid GATTC client logic used by both
// EspHostedBluedroidBLE and NativeBLE. Both use the same IDF
// Bluedroid GATTC API; only the BT controller transport differs.

#include "sdkconfig.h"

#ifdef CONFIG_BT_GATTC_ENABLE

#include <Arduino.h>

#include <cstdint>
#include <functional>
#include <map>
#include <vector>

#include "esp_bt_defs.h"
#include "esp_gatt_defs.h"
#include "esp_gattc_api.h"

#include "sensesp/net/ble/ble_provisioner.h"

namespace sensesp {

/// Holds per-connection state for one GATT connection managed by
/// the Bluedroid GATTC client.
struct GATTConnState {
  String mac;
  String service_uuid;
  uint16_t conn_id = 0;
  uint16_t service_start_handle = 0;
  uint16_t service_end_handle = 0;
  bool service_found = false;
  // Characteristic UUID (lowercase hex) -> attribute handle
  std::map<String, uint16_t> char_handle;
  // Attribute handle -> characteristic UUID (reverse lookup for events)
  std::map<uint16_t, String> handle_to_uuid;
  GATTConnectionCallbacks callbacks;
};

/// Reusable Bluedroid GATTC client. Compose this into a
/// BLEProvisioner implementation and forward the GATTC event
/// callback to handle_gattc_event().
class BluedroidGATTC {
 public:
  static constexpr int kMaxConnections = 3;

  /// Call once after esp_bluedroid_enable() to register GATTC.
  bool init(esp_gattc_cb_t trampoline);

  int max_gatt_connections() const { return kMaxConnections; }
  int active_gatt_connections() const;

  int gatt_connect(const String& mac, uint8_t addr_type,
                   const String& service_uuid,
                   GATTConnectionCallbacks callbacks);
  bool gatt_subscribe_notify(int conn_handle, const String& char_uuid);
  bool gatt_read(int conn_handle, const String& char_uuid);
  bool gatt_write(int conn_handle, const String& char_uuid,
                  const uint8_t* data, size_t len);
  void gatt_disconnect(int conn_handle);

  /// Forward all GATTC events here from the static trampoline.
  void handle_gattc_event(esp_gattc_cb_event_t event,
                          esp_gatt_if_t gattc_if,
                          esp_ble_gattc_cb_param_t* param);

 private:
  esp_gatt_if_t gattc_if_ = ESP_GATT_IF_NONE;

  // Active connections keyed by conn_id.
  std::map<uint16_t, GATTConnState> connections_;

  // Pending connects keyed by MAC (before conn_id is assigned).
  std::map<String, GATTConnState> pending_;

  // Next connection handle to return from gatt_connect().
  // We use conn_id directly as the handle since Bluedroid assigns it.
  // The return value from gatt_connect() is a temporary token;
  // the real conn_id arrives in OPEN_EVT.
  int next_handle_ = 0;

  // Parse "AA:BB:CC:DD:EE:FF" to esp_bd_addr_t.
  static bool parse_mac(const String& mac, esp_bd_addr_t out);

  // Convert a Bluedroid esp_bt_uuid_t to a lowercase hex String.
  static String uuid_to_string(const esp_bt_uuid_t& uuid);

  // Parse a UUID string (16-bit "180f", 128-bit
  // "0000180f-0000-1000-8000-00805f9b34fb") into esp_bt_uuid_t.
  static bool parse_uuid(const String& str, esp_bt_uuid_t& out);

  // Find a connection by conn_id.
  GATTConnState* find_conn(uint16_t conn_id);

  // Discover characteristics after service search completes.
  void discover_characteristics(GATTConnState& conn);
};

}  // namespace sensesp

#endif  // CONFIG_BT_GATTC_ENABLE
#endif  // SENSESP_NET_BLE_BLUEDROID_GATTC_H_
