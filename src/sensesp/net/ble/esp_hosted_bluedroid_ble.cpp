#include "sensesp/net/ble/esp_hosted_bluedroid_ble.h"

#if defined(CONFIG_BT_BLUEDROID_ENABLED) && \
    defined(CONFIG_ESP_HOSTED_ENABLE_BT_BLUEDROID)

#include <string.h>

#include "driver/gpio.h"
#include "esp_bluedroid_hci.h"
#include "esp_bt_device.h"
#include "esp_log.h"

// Arduino-ESP32 helper that brings the ESP-Hosted BT controller up.
// Declared in esp32-hal-hosted.h — on P4 this pulls in esp_hosted's
// SDIO transport and (for slave fw >= 2.6.0) calls
// esp_hosted_bt_controller_init() + _enable() internally.
extern "C" {
#include "esp32-hal-hosted.h"
}

// esp_hosted's Bluedroid VHCI driver functions. These are not
// declared in esp_bluedroid_hci.h (that's the Bluedroid-side
// operations struct, not the transport-side implementation). The
// esp_hosted component ships esp_hosted_bluedroid.h with the
// declarations but it is not on the default include path when we
// build against the library as a managed component, so declare them
// manually here — this matches what esp_hosted's own examples
// (examples/host_bluedroid_host_only/main/main.c) do.
extern "C" {
void hosted_hci_bluedroid_open(void);
void hosted_hci_bluedroid_close(void);
void hosted_hci_bluedroid_send(uint8_t* data, uint16_t len);
bool hosted_hci_bluedroid_check_send_available(void);
esp_err_t hosted_hci_bluedroid_register_host_callback(
    const esp_bluedroid_hci_driver_callbacks_t* callback);

// esp_hosted BT controller lifecycle RPCs. These send RPC commands
// over SDIO to the C6 slave to init/deinit/enable/disable its BT
// controller. Available in esp_hosted >= 2.5.2.
esp_err_t esp_hosted_bt_controller_init(void);
esp_err_t esp_hosted_bt_controller_deinit(bool mem_release);
esp_err_t esp_hosted_bt_controller_enable(void);
esp_err_t esp_hosted_bt_controller_disable(void);
}

namespace sensesp {

namespace {
constexpr const char* kTag = "ble_prov";

// Convert a millisecond scan interval/window to the BT spec units
// (0.625 ms per unit). Clamped to the valid range [0x0004, 0x4000]
// per Bluetooth Core Spec.
uint16_t ms_to_scan_units(uint32_t ms) {
  uint32_t units = (ms * 1000U) / 625U;
  if (units < 0x0004U) {
    units = 0x0004U;
  } else if (units > 0x4000U) {
    units = 0x4000U;
  }
  return static_cast<uint16_t>(units);
}

// Format a 6-byte BD address as "AA:BB:CC:DD:EE:FF".
String format_bda(const uint8_t* bda) {
  char buf[18];
  snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X", bda[0], bda[1],
           bda[2], bda[3], bda[4], bda[5]);
  return String(buf);
}

// Extract the complete-local-name AD field from an adv data payload,
// or return an empty String if not present.
String extract_local_name(const uint8_t* adv_data, size_t len) {
  uint8_t name_len = 0;
  uint8_t* name = esp_ble_resolve_adv_data(const_cast<uint8_t*>(adv_data),
                                           ESP_BLE_AD_TYPE_NAME_CMPL, &name_len);
  if (name != nullptr && name_len > 0) {
    // esp_ble_resolve_adv_data returns a pointer into the buffer, not a
    // null-terminated string.
    char tmp[32];
    size_t copy = name_len < sizeof(tmp) - 1 ? name_len : sizeof(tmp) - 1;
    memcpy(tmp, name, copy);
    tmp[copy] = '\0';
    return String(tmp);
  }
  return String("");
}

}  // namespace

EspHostedBluedroidBLE* EspHostedBluedroidBLE::instance_ = nullptr;

EspHostedBluedroidBLE::EspHostedBluedroidBLE(
    const EspHostedBluedroidBLEConfig& config)
    : config_(config) {
  if (instance_ != nullptr) {
    ESP_LOGE(kTag,
             "Another EspHostedBluedroidBLE instance already exists. "
             "Bluedroid's GAP callback is process-global; only one "
             "instance is supported at a time.");
    return;
  }
  instance_ = this;

  if (config_.enable_hci_logging) {
    esp_log_level_set("BT_HCI", ESP_LOG_VERBOSE);
    esp_log_level_set("vhci_drv", ESP_LOG_VERBOSE);
    esp_log_level_set("transport", ESP_LOG_VERBOSE);
    esp_log_level_set("H_BT", ESP_LOG_VERBOSE);
    ESP_LOGW(kTag, "HCI verbose logging enabled — expect heavy output");
  }

  ESP_LOGI(kTag, "Initialising ESP-Hosted BT controller");
  if (!hostedInitBLE()) {
    ESP_LOGE(kTag, "hostedInitBLE() failed — C6 slave not responding?");
    return;
  }

  if (!bringup_bluedroid()) {
    return;
  }

  // Build the legacy scan parameters struct. We explicitly set
  // CONFIG_BT_BLE_42_FEATURES_SUPPORTED=y in sdkconfig.defaults so
  // that the legacy esp_ble_gap_set_scan_params / start_scanning
  // entry points are compiled in. ESPHome's working P4+C6
  // bluetooth_proxy uses legacy scan exclusively — the C6 slave
  // firmware may not correctly forward BLE 5.0 extended HCI
  // commands over the SDIO bridge.
  const uint16_t itvl = ms_to_scan_units(config_.scan_interval_ms);
  const uint16_t win = ms_to_scan_units(config_.scan_window_ms);
  const esp_ble_scan_type_t type =
      config_.active_scan ? BLE_SCAN_TYPE_ACTIVE : BLE_SCAN_TYPE_PASSIVE;

  scan_params_ = {
      .scan_type = type,
      .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
      .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
      .scan_interval = itvl,
      .scan_window = win,
      .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE,
  };

  bt_stack_up_.store(true);
  ESP_LOGI(kTag,
           "Bluedroid host stack up. active=%d itvl=%ums win=%ums. Call "
           "start_scan() to begin scanning.",
           static_cast<int>(config_.active_scan),
           static_cast<unsigned>(config_.scan_interval_ms),
           static_cast<unsigned>(config_.scan_window_ms));
}

EspHostedBluedroidBLE::~EspHostedBluedroidBLE() {
  if (scanning_.load()) {
    esp_ble_gap_stop_scanning();
  }
  // Intentionally not calling esp_bluedroid_disable() / _deinit()
  // here. Arduino-ESP32's ETH teardown has a similar comment: the
  // IDF teardown paths for stateful subsystems do not reliably
  // restore an initial state that a subsequent re-init can build
  // on, so SensESP provisioners are expected to live as long as
  // the app does.
  instance_ = nullptr;
}

bool EspHostedBluedroidBLE::start_scan() {
  if (!bt_stack_up_.load()) {
    ESP_LOGW(kTag, "start_scan() called but BT stack is not up");
    return false;
  }
  if (scanning_.load()) {
    return true;
  }

  // Set scan params first. The actual scan start happens in the
  // SCAN_PARAM_SET_COMPLETE event handler, to ensure the controller
  // has accepted the params before we try to start.
  esp_err_t err = esp_ble_gap_set_scan_params(&scan_params_);
  if (err != ESP_OK) {
    ESP_LOGE(kTag, "esp_ble_gap_set_scan_params failed: %s",
             esp_err_to_name(err));
    return false;
  }
  return true;
}

bool EspHostedBluedroidBLE::stop_scan() {
  if (!scanning_.load()) {
    return true;
  }
  esp_err_t err = esp_ble_gap_stop_scanning();
  if (err != ESP_OK) {
    ESP_LOGE(kTag, "esp_ble_gap_stop_scanning failed: %s",
             esp_err_to_name(err));
    return false;
  }
  // scanning_ flag is cleared in the STOP_COMPLETE event handler.
  return true;
}

void EspHostedBluedroidBLE::teardown_bluedroid() {
  scanning_.store(false);
  scan_params_set_.store(false);
  bt_stack_up_.store(false);

  esp_err_t err = esp_bluedroid_disable();
  if (err != ESP_OK) {
    ESP_LOGE(kTag, "esp_bluedroid_disable: %s", esp_err_to_name(err));
  }
  err = esp_bluedroid_deinit();
  if (err != ESP_OK) {
    ESP_LOGE(kTag, "esp_bluedroid_deinit: %s", esp_err_to_name(err));
  }
  hosted_hci_bluedroid_close();
}

bool EspHostedBluedroidBLE::bringup_bluedroid() {
  ESP_LOGI(kTag, "Opening hosted HCI VHCI transport");
  hosted_hci_bluedroid_open();

  static const esp_bluedroid_hci_driver_operations_t kHostedHciOps = {
      .send = hosted_hci_bluedroid_send,
      .check_send_available = hosted_hci_bluedroid_check_send_available,
      .register_host_callback = hosted_hci_bluedroid_register_host_callback,
  };
  esp_err_t err = esp_bluedroid_attach_hci_driver(&kHostedHciOps);
  if (err != ESP_OK) {
    ESP_LOGE(kTag, "esp_bluedroid_attach_hci_driver: %s",
             esp_err_to_name(err));
    return false;
  }

  ESP_LOGI(kTag, "Initialising Bluedroid host stack");
  err = esp_bluedroid_init();
  if (err != ESP_OK) {
    ESP_LOGE(kTag, "esp_bluedroid_init: %s", esp_err_to_name(err));
    return false;
  }
  err = esp_bluedroid_enable();
  if (err != ESP_OK) {
    ESP_LOGE(kTag, "esp_bluedroid_enable: %s", esp_err_to_name(err));
    return false;
  }
  err = esp_ble_gap_register_callback(
      &EspHostedBluedroidBLE::gap_event_trampoline);
  if (err != ESP_OK) {
    ESP_LOGE(kTag, "esp_ble_gap_register_callback: %s",
             esp_err_to_name(err));
    return false;
  }

#ifdef CONFIG_BT_GATTC_ENABLE
  if (!gattc_.init(&EspHostedBluedroidBLE::gattc_event_trampoline)) {
    ESP_LOGW(kTag, "GATTC init failed — GATT client will not be available");
    // Non-fatal: scanning still works without GATTC.
  }
#endif

  return true;
}

bool EspHostedBluedroidBLE::reset_bt_controller() {
  ESP_LOGW(kTag, "Resetting BT controller via RPC — Bluedroid + C6 BT restart");

  teardown_bluedroid();

  // Disable and deinit the C6's BT controller via RPC over SDIO.
  // mem_release=false so the controller can be re-inited.
  esp_err_t err = esp_hosted_bt_controller_disable();
  if (err != ESP_OK) {
    ESP_LOGE(kTag, "esp_hosted_bt_controller_disable: %s",
             esp_err_to_name(err));
  }
  err = esp_hosted_bt_controller_deinit(false);
  if (err != ESP_OK) {
    ESP_LOGE(kTag, "esp_hosted_bt_controller_deinit: %s",
             esp_err_to_name(err));
  }

  vTaskDelay(pdMS_TO_TICKS(200));

  // Re-init the C6 BT controller.
  err = esp_hosted_bt_controller_init();
  if (err != ESP_OK) {
    ESP_LOGE(kTag, "esp_hosted_bt_controller_init: %s",
             esp_err_to_name(err));
    return false;
  }
  err = esp_hosted_bt_controller_enable();
  if (err != ESP_OK) {
    ESP_LOGE(kTag, "esp_hosted_bt_controller_enable: %s",
             esp_err_to_name(err));
    return false;
  }

  if (!bringup_bluedroid()) {
    return false;
  }

  bt_stack_up_.store(true);
  ESP_LOGI(kTag, "BT controller RPC reset complete — stack ready for scan");
  return true;
}

bool EspHostedBluedroidBLE::hard_reset_c6() {
  // GPIO hard-reset of the C6 companion chip. This is the nuclear
  // option: it power-cycles the entire C6, killing both its BLE and
  // WiFi stacks. On the Waveshare P4-POE board the C6 reset line is
  // GPIO 54 (CONFIG_ESP_HOSTED_GPIO_SLAVE_RESET_SLAVE). The esp_hosted
  // SDIO transport layer has its own reconnection logic that kicks in
  // when the C6 comes back.
  //
  // This is safe on Ethernet-only boards (no WiFi dependency on C6).
  // On boards that use the C6 for WiFi this would drop the network —
  // the caller should only use this when WiFi is not the primary link.

  constexpr gpio_num_t kC6ResetPin =
      static_cast<gpio_num_t>(CONFIG_ESP_HOSTED_GPIO_SLAVE_RESET_SLAVE);

  ESP_LOGW(kTag,
           "Hard-resetting C6 via GPIO %d — full chip power cycle",
           static_cast<int>(kC6ResetPin));

  // 1. Tear down Bluedroid before yanking the transport.
  teardown_bluedroid();

  // 2. Pulse the C6 reset line. The reset is active-high on the
  //    Waveshare board (CONFIG_ESP_HOSTED_SDIO_RESET_ACTIVE_HIGH=y):
  //    driving HIGH asserts reset, LOW deasserts.
  gpio_set_direction(kC6ResetPin, GPIO_MODE_OUTPUT);
  // Assert reset (HIGH = C6 held in reset).
  gpio_set_level(kC6ResetPin, 1);
  vTaskDelay(pdMS_TO_TICKS(100));
  // Deassert reset (LOW = C6 starts booting).
  gpio_set_level(kC6ResetPin, 0);

  // 3. Wait for the C6 to boot its slave firmware. The esp_hosted
  //    SDIO transport uses a 1500ms timeout for card init after reset.
  //    Give a bit more headroom.
  ESP_LOGI(kTag, "Waiting 2000ms for C6 to boot...");
  vTaskDelay(pdMS_TO_TICKS(2000));

  // 4. Re-init the C6 BT controller via RPC. The SDIO transport
  //    should have reconnected by now.
  esp_err_t err = esp_hosted_bt_controller_init();
  if (err != ESP_OK) {
    ESP_LOGE(kTag, "esp_hosted_bt_controller_init after GPIO reset: %s",
             esp_err_to_name(err));
    return false;
  }
  err = esp_hosted_bt_controller_enable();
  if (err != ESP_OK) {
    ESP_LOGE(kTag, "esp_hosted_bt_controller_enable after GPIO reset: %s",
             esp_err_to_name(err));
    return false;
  }

  // 5. Bring Bluedroid back up on top of the fresh C6.
  if (!bringup_bluedroid()) {
    return false;
  }

  bt_stack_up_.store(true);
  ESP_LOGI(kTag, "C6 GPIO hard-reset complete — stack ready for scan");
  return true;
}

bool EspHostedBluedroidBLE::is_scanning() const { return scanning_.load(); }

String EspHostedBluedroidBLE::mac_address() const {
  if (!bt_stack_up_.load()) {
    return String("");
  }
  const uint8_t* bda = esp_bt_dev_get_address();
  if (bda == nullptr) {
    return String("");
  }
  return format_bda(bda);
}

void EspHostedBluedroidBLE::gap_event_trampoline(
    esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param) {
  if (instance_ == nullptr) {
    return;
  }
  instance_->handle_gap_event(event, param);
}

void EspHostedBluedroidBLE::handle_gap_event(
    esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param) {
  switch (event) {
    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: {
      ESP_LOGI(kTag, "SCAN_PARAM_SET_COMPLETE status=%d",
               param->scan_param_cmpl.status);
      if (param->scan_param_cmpl.status != ESP_BT_STATUS_SUCCESS) {
        ESP_LOGE(kTag, "SCAN_PARAM_SET failed, status=%d",
                 param->scan_param_cmpl.status);
        return;
      }
      scan_params_set_.store(true);
      // duration=0 → scan forever until stopped.
      esp_err_t err = esp_ble_gap_start_scanning(0);
      if (err != ESP_OK) {
        ESP_LOGE(kTag, "esp_ble_gap_start_scanning failed: %s",
                 esp_err_to_name(err));
      }
      break;
    }
    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT: {
      ESP_LOGI(kTag, "SCAN_START_COMPLETE status=%d",
               param->scan_start_cmpl.status);
      if (param->scan_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
        ESP_LOGE(kTag, "SCAN_START failed, status=%d",
                 param->scan_start_cmpl.status);
        return;
      }
      scanning_.store(true);
      ESP_LOGI(kTag, "BLE scan started");
      break;
    }
    case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT: {
      scanning_.store(false);
      ESP_LOGI(kTag, "BLE scan stopped");
      break;
    }
    case ESP_GAP_BLE_SCAN_RESULT_EVT: {
      const auto& r = param->scan_rst;
      if (r.search_evt != ESP_GAP_SEARCH_INQ_RES_EVT) {
        // Other sub-events (INQ_CMPL, DISC_RES, etc.) — ignore.
        break;
      }
      scan_hit_count_.fetch_add(1, std::memory_order_relaxed);

      BLEAdvertisement ad;
      ad.address = format_bda(r.bda);
      ad.address_type = static_cast<uint8_t>(r.ble_addr_type);
      ad.rssi = r.rssi;
      ad.name = extract_local_name(r.ble_adv, r.adv_data_len);
      ad.adv_data.assign(r.ble_adv, r.ble_adv + r.adv_data_len);
      ad.received_at_ms = millis();

      // Emit through the inherited ValueProducer<BLEAdvertisement>
      // so any observer that connect_to'd us receives it.
      this->emit(ad);
      break;
    }
    default:
      ESP_LOGD(kTag, "GAP event %d", static_cast<int>(event));
      break;
  }
}

#ifdef CONFIG_BT_GATTC_ENABLE
void EspHostedBluedroidBLE::gattc_event_trampoline(
    esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
    esp_ble_gattc_cb_param_t* param) {
  if (instance_) {
    instance_->gattc_.handle_gattc_event(event, gattc_if, param);
  }
}
#endif

}  // namespace sensesp

#endif  // CONFIG_BT_BLUEDROID_ENABLED && CONFIG_ESP_HOSTED_ENABLE_BT_BLUEDROID
