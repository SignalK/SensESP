#include "sensesp/net/ble/native_bluedroid_ble.h"

#if defined(CONFIG_BT_BLUEDROID_ENABLED) && \
    !defined(CONFIG_ESP_HOSTED_ENABLE_BT_BLUEDROID)

#include <string.h>

#include "esp_bt.h"
#include "esp_bt_device.h"
#include "esp_log.h"

namespace sensesp {

namespace {
constexpr const char* kTag = "ble_native";

uint16_t ms_to_scan_units(uint32_t ms) {
  uint32_t units = (ms * 1000U) / 625U;
  if (units < 0x0004U) units = 0x0004U;
  else if (units > 0x4000U) units = 0x4000U;
  return static_cast<uint16_t>(units);
}

String format_bda(const uint8_t* bda) {
  char buf[18];
  snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
           bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
  return String(buf);
}

String extract_local_name(const uint8_t* adv_data, size_t len) {
  uint8_t name_len = 0;
  uint8_t* name = esp_ble_resolve_adv_data(
      const_cast<uint8_t*>(adv_data), ESP_BLE_AD_TYPE_NAME_CMPL, &name_len);
  if (name && name_len > 0) {
    char tmp[32];
    size_t copy = name_len < sizeof(tmp) - 1 ? name_len : sizeof(tmp) - 1;
    memcpy(tmp, name, copy);
    tmp[copy] = '\0';
    return String(tmp);
  }
  return String("");
}

}  // namespace

NativeBLE* NativeBLE::instance_ = nullptr;

NativeBLE::NativeBLE(const NativeBLEConfig& config)
    : config_(config) {
  if (instance_) {
    ESP_LOGE(kTag, "Only one NativeBLE instance allowed");
    return;
  }
  instance_ = this;

  ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  esp_err_t err = esp_bt_controller_init(&bt_cfg);
  if (err != ESP_OK) {
    ESP_LOGE(kTag, "bt_controller_init: %s", esp_err_to_name(err));
    return;
  }
  err = esp_bt_controller_enable(ESP_BT_MODE_BLE);
  if (err != ESP_OK) {
    ESP_LOGE(kTag, "bt_controller_enable: %s", esp_err_to_name(err));
    return;
  }

  err = esp_bluedroid_init();
  if (err != ESP_OK) {
    ESP_LOGE(kTag, "bluedroid_init: %s", esp_err_to_name(err));
    return;
  }
  err = esp_bluedroid_enable();
  if (err != ESP_OK) {
    ESP_LOGE(kTag, "bluedroid_enable: %s", esp_err_to_name(err));
    return;
  }

  err = esp_ble_gap_register_callback(&NativeBLE::gap_event_trampoline);
  if (err != ESP_OK) {
    ESP_LOGE(kTag, "gap_register_callback: %s", esp_err_to_name(err));
    return;
  }

#ifdef CONFIG_BT_GATTC_ENABLE
  if (!gattc_.init(&NativeBLE::gattc_event_trampoline)) {
    ESP_LOGW(kTag, "GATTC init failed — GATT client will not be available");
  }
#endif

  const uint16_t itvl = ms_to_scan_units(config_.scan_interval_ms);
  const uint16_t win = ms_to_scan_units(config_.scan_window_ms);

  scan_params_ = {
      .scan_type = config_.active_scan ? BLE_SCAN_TYPE_ACTIVE
                                       : BLE_SCAN_TYPE_PASSIVE,
      .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
      .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
      .scan_interval = itvl,
      .scan_window = win,
      .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE,
  };

  bt_stack_up_.store(true);
  ESP_LOGI(kTag, "Bluedroid up. active=%d itvl=%ums win=%ums",
           (int)config_.active_scan,
           (unsigned)config_.scan_interval_ms,
           (unsigned)config_.scan_window_ms);
}

NativeBLE::~NativeBLE() {
  if (scanning_.load()) esp_ble_gap_stop_scanning();
  instance_ = nullptr;
}

bool NativeBLE::start_scan() {
  if (!bt_stack_up_.load()) return false;
  if (scanning_.load()) return true;

  esp_err_t err = esp_ble_gap_set_scan_params(&scan_params_);
  if (err != ESP_OK) {
    ESP_LOGE(kTag, "set_scan_params: %s", esp_err_to_name(err));
    return false;
  }
  return true;
}

bool NativeBLE::stop_scan() {
  if (!scanning_.load()) return true;
  esp_err_t err = esp_ble_gap_stop_scanning();
  if (err != ESP_OK) {
    ESP_LOGE(kTag, "stop_scanning: %s", esp_err_to_name(err));
    return false;
  }
  return true;
}

bool NativeBLE::is_scanning() const { return scanning_.load(); }

String NativeBLE::mac_address() const {
  if (!bt_stack_up_.load()) return String("");
  const uint8_t* bda = esp_bt_dev_get_address();
  return bda ? format_bda(bda) : String("");
}

void NativeBLE::gap_event_trampoline(
    esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param) {
  if (instance_) instance_->handle_gap_event(event, param);
}

void NativeBLE::handle_gap_event(
    esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param) {
  switch (event) {
    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
      if (param->scan_param_cmpl.status == ESP_BT_STATUS_SUCCESS) {
        esp_ble_gap_start_scanning(0);
      } else {
        ESP_LOGE(kTag, "SCAN_PARAM_SET failed: %d",
                 param->scan_param_cmpl.status);
      }
      break;
    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
      if (param->scan_start_cmpl.status == ESP_BT_STATUS_SUCCESS) {
        scanning_.store(true);
        ESP_LOGI(kTag, "BLE scan started");
      } else {
        ESP_LOGE(kTag, "SCAN_START failed: %d",
                 param->scan_start_cmpl.status);
      }
      break;
    case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
      scanning_.store(false);
      ESP_LOGI(kTag, "BLE scan stopped");
      break;
    case ESP_GAP_BLE_SCAN_RESULT_EVT: {
      const auto& r = param->scan_rst;
      if (r.search_evt != ESP_GAP_SEARCH_INQ_RES_EVT) break;
      scan_hit_count_.fetch_add(1, std::memory_order_relaxed);

      BLEAdvertisement ad;
      ad.address = format_bda(r.bda);
      ad.address_type = static_cast<uint8_t>(r.ble_addr_type);
      ad.rssi = r.rssi;
      ad.name = extract_local_name(r.ble_adv, r.adv_data_len);
      ad.adv_data.assign(r.ble_adv, r.ble_adv + r.adv_data_len);
      ad.received_at_ms = millis();
      this->emit(ad);
      break;
    }
    default:
      break;
  }
}

#ifdef CONFIG_BT_GATTC_ENABLE
void NativeBLE::gattc_event_trampoline(
    esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
    esp_ble_gattc_cb_param_t* param) {
  if (instance_) {
    instance_->gattc_.handle_gattc_event(event, gattc_if, param);
  }
}
#endif

}  // namespace sensesp

#endif
