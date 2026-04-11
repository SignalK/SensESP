/**
 * @file main.cpp
 * @brief SensESP P4 BLE gateway example.
 *
 * BLE gateway on the Waveshare ESP32-P4-WIFI6-POE-ETH board:
 *
 *   - RMII Ethernet for connectivity (EthernetProvisioner)
 *   - EspHostedBluedroidBLE for BLE scanning via the onboard C6
 *   - BLESignalKGateway bridges BLE to signalk-server's provider API
 *
 * The C6 companion chip handles BLE over SDIO. An external 2.4 GHz
 * antenna must be connected to the C6 module's IPEX connector (the
 * ESP32-C6-MINI-1U variant has no built-in PCB antenna).
 */

#include "sensesp/net/ble/ble_signalk_gateway.h"
#include "sensesp/net/ble/esp_hosted_bluedroid_ble.h"
#include "sensesp/net/ethernet_provisioner.h"
#include "sensesp_app_builder.h"

#include "SPIFFS.h"

extern "C" {
#include "esp32-hal-hosted.h"
esp_err_t esp_hosted_slave_ota_begin(void);
esp_err_t esp_hosted_slave_ota_write(uint8_t* data, uint32_t len);
esp_err_t esp_hosted_slave_ota_end(void);
esp_err_t esp_hosted_slave_ota_activate(void);
}

using namespace sensesp;

// Kept alive for the lifetime of the app. The gateway keeps a
// shared_ptr to both, so these could also be function-local — they
// are file-static only so the heartbeat lambda below can touch their
// counters for logging.
static std::shared_ptr<EspHostedBluedroidBLE> g_ble;
static std::shared_ptr<BLESignalKGateway> g_gateway;

// One-shot C6 slave firmware OTA. If /spiffs/c6_fw.bin exists, push
// it to the C6 via esp_hosted's OTA RPC, activate, and reboot. The
// file is deleted after a successful flash so it only runs once.
static void maybe_ota_c6_slave() {
  if (!SPIFFS.begin(false, "/spiffs")) {
    return;  // No SPIFFS — skip silently.
  }
  if (!SPIFFS.exists("/c6_fw.bin")) {
    SPIFFS.end();
    return;  // No firmware staged — normal boot.
  }

  ESP_LOGW("C6_OTA", "Found /spiffs/c6_fw.bin — starting C6 slave OTA");

  // The hosted transport must be up before we can send OTA RPCs.
  // hostedInit() is idempotent — if already initialized by an
  // earlier call (e.g. hostedInitBLE), it returns true immediately.
  if (!hostedInitBLE()) {
    ESP_LOGE("C6_OTA", "hostedInitBLE failed — cannot OTA");
    SPIFFS.end();
    return;
  }

  File fw = SPIFFS.open("/c6_fw.bin", "r");
  if (!fw) {
    ESP_LOGE("C6_OTA", "Failed to open /spiffs/c6_fw.bin");
    SPIFFS.end();
    return;
  }

  size_t total = fw.size();
  ESP_LOGI("C6_OTA", "Firmware size: %u bytes", (unsigned)total);

  esp_err_t err = esp_hosted_slave_ota_begin();
  if (err != ESP_OK) {
    ESP_LOGE("C6_OTA", "ota_begin failed: %s", esp_err_to_name(err));
    fw.close();
    SPIFFS.end();
    return;
  }

  uint8_t buf[1500];
  size_t sent = 0;
  while (fw.available()) {
    size_t n = fw.read(buf, sizeof(buf));
    err = esp_hosted_slave_ota_write(buf, n);
    if (err != ESP_OK) {
      ESP_LOGE("C6_OTA", "ota_write failed at offset %u: %s",
               (unsigned)sent, esp_err_to_name(err));
      fw.close();
      SPIFFS.end();
      return;
    }
    sent += n;
    if (sent % (100 * 1500) < 1500) {
      ESP_LOGI("C6_OTA", "Progress: %u / %u bytes", (unsigned)sent,
               (unsigned)total);
    }
  }
  fw.close();

  ESP_LOGI("C6_OTA", "Sent %u bytes — finalizing OTA", (unsigned)sent);
  err = esp_hosted_slave_ota_end();
  if (err != ESP_OK) {
    ESP_LOGE("C6_OTA", "ota_end failed: %s", esp_err_to_name(err));
    SPIFFS.end();
    return;
  }

  ESP_LOGI("C6_OTA", "Activating new C6 firmware");
  err = esp_hosted_slave_ota_activate();
  if (err != ESP_OK) {
    ESP_LOGE("C6_OTA", "ota_activate failed: %s", esp_err_to_name(err));
    SPIFFS.end();
    return;
  }

  // Delete the firmware file so we don't OTA on every boot.
  SPIFFS.remove("/c6_fw.bin");
  SPIFFS.end();

  ESP_LOGW("C6_OTA", "C6 slave firmware updated — rebooting in 2s");
  delay(2000);
  ESP.restart();
}

void setup() {
  SetupLogging(ESP_LOG_INFO);

  // Check for staged C6 slave firmware and flash it before anything
  // else. This only runs when /spiffs/c6_fw.bin exists.
  maybe_ota_c6_slave();

  SensESPAppBuilder builder;
  auto app = builder.set_hostname(GATEWAY_HOSTNAME)
                 ->set_ethernet(EthernetConfig::waveshare_esp32p4_poe())
                 ->disable_wifi()
                 ->enable_ota("bluedroid-poc-ota")
                 ->get_app();

  g_ble = std::make_shared<EspHostedBluedroidBLE>();

  g_gateway =
      std::make_shared<BLESignalKGateway>(g_ble, app->get_ws_client());
  g_gateway->start();

  event_loop()->onRepeat(5000, []() {
    ESP_LOGI(
        "POC",
        "alive — uptime=%lus heap=%u ble_hits=%u ble_scan=%d gw_rx=%u "
        "gw_posted=%u gw_dropped=%u post_ok=%u post_fail=%u ws_up=%d",
        (unsigned long)(millis() / 1000), (unsigned)ESP.getFreeHeap(),
        (unsigned)(g_ble ? g_ble->scan_hit_count() : 0),
        (int)(g_ble ? g_ble->is_scanning() : false),
        (unsigned)(g_gateway ? g_gateway->advertisements_received() : 0),
        (unsigned)(g_gateway ? g_gateway->advertisements_posted() : 0),
        (unsigned)(g_gateway ? g_gateway->advertisements_dropped() : 0),
        (unsigned)(g_gateway ? g_gateway->http_post_success() : 0),
        (unsigned)(g_gateway ? g_gateway->http_post_fail() : 0),
        (int)(g_gateway ? g_gateway->control_ws_connected() : false));
  });
}

void loop() { event_loop()->tick(); }
