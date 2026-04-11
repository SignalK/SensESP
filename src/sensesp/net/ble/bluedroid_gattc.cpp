#include "sensesp/net/ble/bluedroid_gattc.h"

#ifdef CONFIG_BT_GATTC_ENABLE

#include <cstring>

#include "esp_log.h"

namespace sensesp {

namespace {
constexpr const char* kTag = "gattc";

// Standard BLE Client Characteristic Configuration Descriptor UUID.
constexpr uint16_t kCCCD_UUID = 0x2902;
// Enable notifications value for CCCD write.
constexpr uint8_t kNotifyEnable[] = {0x01, 0x00};
}  // namespace

// ---------------------------------------------------------------
// UUID helpers
// ---------------------------------------------------------------

bool BluedroidGATTC::parse_mac(const String& mac, esp_bd_addr_t out) {
  if (mac.length() != 17) return false;
  unsigned int b[6];
  if (sscanf(mac.c_str(), "%02X:%02X:%02X:%02X:%02X:%02X",
             &b[0], &b[1], &b[2], &b[3], &b[4], &b[5]) != 6) {
    return false;
  }
  for (int i = 0; i < 6; i++) out[i] = static_cast<uint8_t>(b[i]);
  return true;
}

String BluedroidGATTC::uuid_to_string(const esp_bt_uuid_t& uuid) {
  char buf[37];
  if (uuid.len == ESP_UUID_LEN_16) {
    snprintf(buf, sizeof(buf), "%04x", uuid.uuid.uuid16);
  } else if (uuid.len == ESP_UUID_LEN_32) {
    snprintf(buf, sizeof(buf), "%08x", (unsigned)uuid.uuid.uuid32);
  } else {
    const uint8_t* u = uuid.uuid.uuid128;
    // BLE 128-bit UUID is stored little-endian in uuid128[].
    snprintf(buf, sizeof(buf),
             "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-"
             "%02x%02x%02x%02x%02x%02x",
             u[15], u[14], u[13], u[12], u[11], u[10], u[9], u[8],
             u[7], u[6], u[5], u[4], u[3], u[2], u[1], u[0]);
  }
  return String(buf);
}

bool BluedroidGATTC::parse_uuid(const String& str, esp_bt_uuid_t& out) {
  if (str.length() == 4) {
    // 16-bit: "180f"
    out.len = ESP_UUID_LEN_16;
    out.uuid.uuid16 =
        static_cast<uint16_t>(strtoul(str.c_str(), nullptr, 16));
    return true;
  }
  if (str.length() == 8) {
    // 32-bit: "0000180f"
    out.len = ESP_UUID_LEN_32;
    out.uuid.uuid32 = strtoul(str.c_str(), nullptr, 16);
    return true;
  }
  if (str.length() == 36) {
    // 128-bit: "0000180f-0000-1000-8000-00805f9b34fb"
    out.len = ESP_UUID_LEN_128;
    unsigned int b[16];
    if (sscanf(str.c_str(),
               "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-"
               "%02x%02x%02x%02x%02x%02x",
               &b[0], &b[1], &b[2], &b[3], &b[4], &b[5], &b[6], &b[7],
               &b[8], &b[9], &b[10], &b[11], &b[12], &b[13], &b[14],
               &b[15]) != 16) {
      return false;
    }
    // Store little-endian (reverse of the parsed big-endian string).
    for (int i = 0; i < 16; i++) {
      out.uuid.uuid128[15 - i] = static_cast<uint8_t>(b[i]);
    }
    return true;
  }
  return false;
}

GATTConnState* BluedroidGATTC::find_conn(uint16_t conn_id) {
  auto it = connections_.find(conn_id);
  return it != connections_.end() ? &it->second : nullptr;
}

// ---------------------------------------------------------------
// Public API
// ---------------------------------------------------------------

bool BluedroidGATTC::init(esp_gattc_cb_t trampoline) {
  esp_err_t err = esp_ble_gattc_register_callback(trampoline);
  if (err != ESP_OK) {
    ESP_LOGE(kTag, "gattc_register_callback: %s", esp_err_to_name(err));
    return false;
  }
  err = esp_ble_gattc_app_register(0);
  if (err != ESP_OK) {
    ESP_LOGE(kTag, "gattc_app_register: %s", esp_err_to_name(err));
    return false;
  }
  return true;
}

int BluedroidGATTC::active_gatt_connections() const {
  return static_cast<int>(connections_.size());
}

int BluedroidGATTC::gatt_connect(const String& mac, uint8_t addr_type,
                                 const String& service_uuid,
                                 GATTConnectionCallbacks callbacks) {
  if (gattc_if_ == ESP_GATT_IF_NONE) {
    ESP_LOGE(kTag, "GATTC not initialized (no gattc_if)");
    return -1;
  }
  if (static_cast<int>(connections_.size() + pending_.size()) >=
      kMaxConnections) {
    ESP_LOGW(kTag, "Max GATT connections reached");
    return -1;
  }

  esp_bd_addr_t bda;
  if (!parse_mac(mac, bda)) {
    ESP_LOGE(kTag, "Invalid MAC: %s", mac.c_str());
    return -1;
  }

  GATTConnState state;
  state.mac = mac;
  state.service_uuid = service_uuid;
  state.callbacks = std::move(callbacks);

  String mac_upper = mac;
  mac_upper.toUpperCase();
  pending_[mac_upper] = std::move(state);

  esp_ble_addr_type_t bt_addr_type =
      addr_type == 1 ? BLE_ADDR_TYPE_RANDOM : BLE_ADDR_TYPE_PUBLIC;

  esp_err_t err =
      esp_ble_gattc_open(gattc_if_, bda, bt_addr_type, true);
  if (err != ESP_OK) {
    ESP_LOGE(kTag, "gattc_open failed: %s", esp_err_to_name(err));
    pending_.erase(mac_upper);
    return -1;
  }

  ESP_LOGI(kTag, "Connecting to %s...", mac.c_str());
  int handle = next_handle_++;
  return handle;
}

bool BluedroidGATTC::gatt_subscribe_notify(int /*conn_handle*/,
                                           const String& char_uuid) {
  // Find the connection that has this characteristic. Since conn_handle
  // is an opaque token and the real conn_id is in connections_, we
  // search by UUID presence.
  for (auto& [conn_id, conn] : connections_) {
    auto it = conn.char_handle.find(char_uuid);
    if (it == conn.char_handle.end()) continue;

    uint16_t char_handle = it->second;

    esp_err_t err =
        esp_ble_gattc_register_for_notify(gattc_if_, nullptr, char_handle);
    // Note: the BDA parameter is ignored by Bluedroid when
    // char_handle is provided. Pass nullptr.
    // Actually, the API requires a valid BDA. Parse from conn.mac.
    esp_bd_addr_t bda;
    parse_mac(conn.mac, bda);
    err = esp_ble_gattc_register_for_notify(gattc_if_, bda, char_handle);
    if (err != ESP_OK) {
      ESP_LOGE(kTag, "register_for_notify(%s) failed: %s",
               char_uuid.c_str(), esp_err_to_name(err));
      return false;
    }

    // Also write to the CCCD descriptor to enable notifications.
    // Find the CCCD by looking at descriptors of this characteristic.
    uint16_t count = 0;
    esp_gattc_descr_elem_t descr;
    esp_bt_uuid_t cccd_uuid = {.len = ESP_UUID_LEN_16,
                               .uuid = {.uuid16 = kCCCD_UUID}};
    esp_gatt_status_t status = esp_ble_gattc_get_descr_by_char_handle(
        gattc_if_, conn_id, char_handle, cccd_uuid, &descr, &count);
    if (status == ESP_GATT_OK && count > 0) {
      esp_ble_gattc_write_char_descr(
          gattc_if_, conn_id, descr.handle,
          sizeof(kNotifyEnable),
          const_cast<uint8_t*>(kNotifyEnable),
          ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
    }

    ESP_LOGI(kTag, "Subscribed to notifications on %s (handle=%u)",
             char_uuid.c_str(), char_handle);
    return true;
  }
  ESP_LOGW(kTag, "gatt_subscribe_notify: char %s not found",
           char_uuid.c_str());
  return false;
}

bool BluedroidGATTC::gatt_read(int /*conn_handle*/,
                               const String& char_uuid) {
  for (auto& [conn_id, conn] : connections_) {
    auto it = conn.char_handle.find(char_uuid);
    if (it == conn.char_handle.end()) continue;

    esp_err_t err = esp_ble_gattc_read_char(
        gattc_if_, conn_id, it->second, ESP_GATT_AUTH_REQ_NONE);
    if (err != ESP_OK) {
      ESP_LOGE(kTag, "read_char(%s) failed: %s",
               char_uuid.c_str(), esp_err_to_name(err));
      return false;
    }
    return true;
  }
  return false;
}

bool BluedroidGATTC::gatt_write(int /*conn_handle*/,
                                const String& char_uuid,
                                const uint8_t* data, size_t len) {
  for (auto& [conn_id, conn] : connections_) {
    auto it = conn.char_handle.find(char_uuid);
    if (it == conn.char_handle.end()) continue;

    esp_err_t err = esp_ble_gattc_write_char(
        gattc_if_, conn_id, it->second, len,
        const_cast<uint8_t*>(data),
        ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
    if (err != ESP_OK) {
      ESP_LOGE(kTag, "write_char(%s) failed: %s",
               char_uuid.c_str(), esp_err_to_name(err));
      return false;
    }
    return true;
  }
  return false;
}

void BluedroidGATTC::gatt_disconnect(int /*conn_handle*/) {
  // Find the most recent connection and close it.
  // TODO: use conn_handle to identify the specific connection once
  // we have a stable mapping from gatt_connect() return value to conn_id.
  if (!connections_.empty()) {
    auto it = connections_.begin();
    ESP_LOGI(kTag, "Disconnecting from %s (conn_id=%u)",
             it->second.mac.c_str(), it->first);
    esp_ble_gattc_close(gattc_if_, it->first);
  }
}

// ---------------------------------------------------------------
// GATTC event handler
// ---------------------------------------------------------------

void BluedroidGATTC::discover_characteristics(GATTConnState& conn) {
  if (!conn.service_found) {
    String err = "Service " + conn.service_uuid + " not found";
    ESP_LOGE(kTag, "%s", err.c_str());
    if (conn.callbacks.on_error) conn.callbacks.on_error(err);
    return;
  }

  // Use the Bluedroid local cache to enumerate characteristics.
  uint16_t count = 0;
  esp_gatt_status_t status = esp_ble_gattc_get_attr_count(
      gattc_if_, conn.conn_id, ESP_GATT_DB_CHARACTERISTIC,
      conn.service_start_handle, conn.service_end_handle, 0, &count);
  if (status != ESP_GATT_OK || count == 0) {
    ESP_LOGW(kTag, "No characteristics found in service %s",
             conn.service_uuid.c_str());
    if (conn.callbacks.on_connected) conn.callbacks.on_connected();
    return;
  }

  std::vector<esp_gattc_char_elem_t> chars(count);
  status = esp_ble_gattc_get_all_char(
      gattc_if_, conn.conn_id,
      conn.service_start_handle, conn.service_end_handle,
      chars.data(), &count, 0);
  if (status != ESP_GATT_OK) {
    ESP_LOGW(kTag, "get_all_char failed: %d", status);
    if (conn.callbacks.on_connected) conn.callbacks.on_connected();
    return;
  }

  for (uint16_t i = 0; i < count; i++) {
    String uuid_str = uuid_to_string(chars[i].uuid);
    conn.char_handle[uuid_str] = chars[i].char_handle;
    conn.handle_to_uuid[chars[i].char_handle] = uuid_str;
    ESP_LOGI(kTag, "  char: %s handle=%u props=0x%02x",
             uuid_str.c_str(), chars[i].char_handle, chars[i].properties);
  }

  ESP_LOGI(kTag, "Service %s: %u characteristics discovered",
           conn.service_uuid.c_str(), count);
  if (conn.callbacks.on_connected) conn.callbacks.on_connected();
}

void BluedroidGATTC::handle_gattc_event(esp_gattc_cb_event_t event,
                                        esp_gatt_if_t gattc_if,
                                        esp_ble_gattc_cb_param_t* param) {
  switch (event) {
    case ESP_GATTC_REG_EVT: {
      if (param->reg.status == ESP_GATT_OK) {
        gattc_if_ = gattc_if;
        ESP_LOGI(kTag, "GATTC registered, gattc_if=%d", gattc_if);
      } else {
        ESP_LOGE(kTag, "GATTC reg failed: %d", param->reg.status);
      }
      break;
    }

    case ESP_GATTC_OPEN_EVT: {
      if (param->open.status != ESP_GATT_OK) {
        ESP_LOGE(kTag, "GATTC open failed: %d", param->open.status);
        // Find and notify the pending connection.
        char mac_buf[18];
        snprintf(mac_buf, sizeof(mac_buf),
                 "%02X:%02X:%02X:%02X:%02X:%02X",
                 param->open.remote_bda[0], param->open.remote_bda[1],
                 param->open.remote_bda[2], param->open.remote_bda[3],
                 param->open.remote_bda[4], param->open.remote_bda[5]);
        String mac_str(mac_buf);
        auto it = pending_.find(mac_str);
        if (it != pending_.end()) {
          if (it->second.callbacks.on_error) {
            it->second.callbacks.on_error(
                String("GATT open failed: ") + String(param->open.status));
          }
          pending_.erase(it);
        }
        break;
      }

      // Move from pending to active connections.
      char mac_buf[18];
      snprintf(mac_buf, sizeof(mac_buf),
               "%02X:%02X:%02X:%02X:%02X:%02X",
               param->open.remote_bda[0], param->open.remote_bda[1],
               param->open.remote_bda[2], param->open.remote_bda[3],
               param->open.remote_bda[4], param->open.remote_bda[5]);
      String mac_str(mac_buf);

      auto it = pending_.find(mac_str);
      if (it == pending_.end()) {
        ESP_LOGW(kTag, "OPEN_EVT for unknown MAC %s", mac_buf);
        break;
      }

      uint16_t conn_id = param->open.conn_id;
      GATTConnState conn = std::move(it->second);
      conn.conn_id = conn_id;
      pending_.erase(it);
      connections_[conn_id] = std::move(conn);

      ESP_LOGI(kTag, "Connected to %s, conn_id=%u, requesting MTU",
               mac_buf, conn_id);
      esp_ble_gattc_send_mtu_req(gattc_if, conn_id);
      break;
    }

    case ESP_GATTC_CFG_MTU_EVT: {
      if (param->cfg_mtu.status != ESP_GATT_OK) {
        ESP_LOGW(kTag, "MTU config failed: %d", param->cfg_mtu.status);
      }
      auto* conn = find_conn(param->cfg_mtu.conn_id);
      if (!conn) break;

      ESP_LOGI(kTag, "MTU=%u, discovering service %s",
               param->cfg_mtu.mtu, conn->service_uuid.c_str());

      esp_bt_uuid_t filter;
      if (parse_uuid(conn->service_uuid, filter)) {
        esp_ble_gattc_search_service(gattc_if, conn->conn_id, &filter);
      } else {
        // No filter — discover all services.
        esp_ble_gattc_search_service(gattc_if, conn->conn_id, nullptr);
      }
      break;
    }

    case ESP_GATTC_SEARCH_RES_EVT: {
      auto* conn = find_conn(param->search_res.conn_id);
      if (!conn) break;

      String found_uuid = uuid_to_string(param->search_res.srvc_id.uuid);
      ESP_LOGI(kTag, "Found service: %s [%u-%u]",
               found_uuid.c_str(),
               param->search_res.start_handle,
               param->search_res.end_handle);

      // Check if this matches our target service.
      String target = conn->service_uuid;
      target.toLowerCase();
      found_uuid.toLowerCase();
      if (found_uuid == target || found_uuid.indexOf(target) >= 0 ||
          target.indexOf(found_uuid) >= 0) {
        conn->service_start_handle = param->search_res.start_handle;
        conn->service_end_handle = param->search_res.end_handle;
        conn->service_found = true;
      }
      break;
    }

    case ESP_GATTC_SEARCH_CMPL_EVT: {
      auto* conn = find_conn(param->search_cmpl.conn_id);
      if (!conn) break;
      discover_characteristics(*conn);
      break;
    }

    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
      if (param->reg_for_notify.status != ESP_GATT_OK) {
        ESP_LOGW(kTag, "REG_FOR_NOTIFY failed: %d",
                 param->reg_for_notify.status);
      }
      break;
    }

    case ESP_GATTC_NOTIFY_EVT: {
      auto* conn = find_conn(param->notify.conn_id);
      if (!conn || !conn->callbacks.on_notify) break;

      auto it = conn->handle_to_uuid.find(param->notify.handle);
      if (it == conn->handle_to_uuid.end()) break;

      conn->callbacks.on_notify(it->second,
                                param->notify.value,
                                param->notify.value_len);
      break;
    }

    case ESP_GATTC_READ_CHAR_EVT: {
      auto* conn = find_conn(param->read.conn_id);
      if (!conn) break;

      auto it = conn->handle_to_uuid.find(param->read.handle);
      String uuid = it != conn->handle_to_uuid.end() ? it->second : "";

      if (param->read.status == ESP_GATT_OK && conn->callbacks.on_read) {
        conn->callbacks.on_read(uuid, param->read.value,
                                param->read.value_len);
      }
      break;
    }

    case ESP_GATTC_WRITE_CHAR_EVT:
    case ESP_GATTC_WRITE_DESCR_EVT: {
      auto* conn = find_conn(param->write.conn_id);
      if (!conn || !conn->callbacks.on_write_complete) break;

      auto it = conn->handle_to_uuid.find(param->write.handle);
      String uuid = it != conn->handle_to_uuid.end() ? it->second : "";
      conn->callbacks.on_write_complete(
          uuid, param->write.status == ESP_GATT_OK);
      break;
    }

    case ESP_GATTC_DISCONNECT_EVT: {
      uint16_t conn_id = param->disconnect.conn_id;
      auto it = connections_.find(conn_id);
      if (it == connections_.end()) break;

      ESP_LOGI(kTag, "Disconnected from %s, reason=%d",
               it->second.mac.c_str(), param->disconnect.reason);
      if (it->second.callbacks.on_disconnected) {
        it->second.callbacks.on_disconnected(
            String("reason:") + String(param->disconnect.reason));
      }
      connections_.erase(it);
      break;
    }

    default:
      ESP_LOGD(kTag, "GATTC event %d", static_cast<int>(event));
      break;
  }
}

}  // namespace sensesp

#endif  // CONFIG_BT_GATTC_ENABLE
