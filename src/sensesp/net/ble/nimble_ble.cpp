#include "sensesp/net/ble/nimble_ble.h"

#if defined(CONFIG_BT_NIMBLE_ENABLED) && \
    !defined(CONFIG_ESP_HOSTED_ENABLE_BT_NIMBLE)

#include <cstring>

#include "esp_log.h"
#include "host/ble_gap.h"
#include "host/ble_hs.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"

namespace sensesp {

namespace {
constexpr const char* kTag = "ble_nimble";

String format_addr(const uint8_t* addr) {
  char buf[18];
  snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
           addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
  return String(buf);
}

String extract_name(const struct ble_hs_adv_fields& fields) {
  if (fields.name != nullptr && fields.name_len > 0) {
    char tmp[32];
    size_t copy = fields.name_len < sizeof(tmp) - 1
                      ? fields.name_len
                      : sizeof(tmp) - 1;
    memcpy(tmp, fields.name, copy);
    tmp[copy] = '\0';
    return String(tmp);
  }
  return String("");
}

// NimBLE host task — runs the NimBLE host stack.
void nimble_host_task(void* param) {
  nimble_port_run();
  nimble_port_freertos_deinit();
}

}  // namespace

NimBLEProvisioner* NimBLEProvisioner::instance_ = nullptr;

NimBLEProvisioner::NimBLEProvisioner(const NimBLEProvisionerConfig& config)
    : config_(config) {
  if (instance_) {
    ESP_LOGE(kTag, "Only one NimBLEProvisioner instance allowed");
    return;
  }
  instance_ = this;

  esp_err_t err = nimble_port_init();
  if (err != ESP_OK) {
    ESP_LOGE(kTag, "nimble_port_init failed: %s", esp_err_to_name(err));
    return;
  }

  // Configure the NimBLE host.
  ble_hs_cfg.sync_cb = []() {
    ESP_LOGI(kTag, "NimBLE host synced");
  };
  ble_hs_cfg.reset_cb = [](int reason) {
    ESP_LOGW(kTag, "NimBLE host reset, reason=%d", reason);
  };

  nimble_port_freertos_init(nimble_host_task);

  ESP_LOGI(kTag, "NimBLE up. active=%d itvl=%ums win=%ums",
           (int)config_.active_scan,
           (unsigned)config_.scan_interval_ms,
           (unsigned)config_.scan_window_ms);
}

NimBLEProvisioner::~NimBLEProvisioner() {
  if (scanning_.load()) {
    ble_gap_disc_cancel();
  }
  instance_ = nullptr;
}

bool NimBLEProvisioner::start_scan() {
  if (scanning_.load()) return true;

  struct ble_gap_disc_params params = {};
  params.passive = config_.active_scan ? 0 : 1;
  params.itvl = (config_.scan_interval_ms * 1000) / 625;
  params.window = (config_.scan_window_ms * 1000) / 625;
  params.filter_duplicates = 0;
  params.limited = 0;
  params.filter_policy = BLE_HCI_SCAN_FILT_NO_WL;

  int rc = ble_gap_disc(BLE_OWN_ADDR_PUBLIC, BLE_HS_FOREVER, &params,
                        &NimBLEProvisioner::gap_event_handler, nullptr);
  if (rc != 0) {
    ESP_LOGE(kTag, "ble_gap_disc failed: %d", rc);
    return false;
  }
  scanning_.store(true);
  ESP_LOGI(kTag, "BLE scan started");
  return true;
}

bool NimBLEProvisioner::stop_scan() {
  if (!scanning_.load()) return true;
  int rc = ble_gap_disc_cancel();
  if (rc != 0 && rc != BLE_HS_EALREADY) {
    ESP_LOGE(kTag, "ble_gap_disc_cancel failed: %d", rc);
    return false;
  }
  scanning_.store(false);
  ESP_LOGI(kTag, "BLE scan stopped");
  return true;
}

bool NimBLEProvisioner::is_scanning() const { return scanning_.load(); }

String NimBLEProvisioner::mac_address() const {
  uint8_t addr[6];
  uint8_t type;
  ble_hs_id_infer_auto(0, &type);
  int rc = ble_hs_id_copy_addr(type, addr, nullptr);
  if (rc != 0) return String("");
  return format_addr(addr);
}

int NimBLEProvisioner::gap_event_handler(struct ble_gap_event* event,
                                         void* /*arg*/) {
  if (!instance_) return 0;

  switch (event->type) {
    case BLE_GAP_EVENT_DISC: {
      instance_->scan_hit_count_.fetch_add(1, std::memory_order_relaxed);

      const struct ble_gap_disc_desc& desc = event->disc;

      BLEAdvertisement ad;
      ad.address = format_addr(desc.addr.val);
      ad.address_type = desc.addr.type;
      ad.rssi = desc.rssi;

      // Parse advertisement data for name.
      struct ble_hs_adv_fields fields;
      if (ble_hs_adv_parse_fields(&fields, desc.data, desc.length_data) == 0) {
        ad.name = extract_name(fields);
      }

      ad.adv_data.assign(desc.data, desc.data + desc.length_data);
      ad.received_at_ms = millis();

      instance_->emit(ad);
      break;
    }
    case BLE_GAP_EVENT_DISC_COMPLETE: {
      ESP_LOGI(kTag, "Scan complete (reason=%d)", event->disc_complete.reason);
      instance_->scanning_.store(false);
      break;
    }
    default:
      break;
  }
  return 0;
}

}  // namespace sensesp

#endif  // CONFIG_BT_NIMBLE_ENABLED && !CONFIG_ESP_HOSTED_ENABLE_BT_NIMBLE
