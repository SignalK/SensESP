/**
 * @file main.cpp
 * @brief SensESP BLE gateway on ESP32-C5 (NimBLE + WiFi).
 *
 * Uses NimBLE instead of Bluedroid to fit WiFi + BLE + HTTP/WS
 * within the C5's limited internal SRAM. Scan duty cycle and
 * ad buffer are tuned to keep heap above ~10KB.
 */

#include "sensesp/net/ble/ble_signalk_gateway.h"
#include "sensesp/net/ble/nimble_ble.h"
#include "sensesp_app_builder.h"

using namespace sensesp;

static std::shared_ptr<NimBLEProvisioner> g_ble;
static std::shared_ptr<BLESignalKGateway> g_gateway;

void setup() {
  SetupLogging(ESP_LOG_INFO);

  SensESPAppBuilder builder;
  auto app = builder.set_hostname(GATEWAY_HOSTNAME)
                 ->set_wifi_client("MOIN", "Moin2018!")
                 ->enable_ota("c5-ble-gw-ota")
                 ->get_app();

  // Lower scan duty cycle to reduce memory pressure on the C5
  // (WiFi + BLE + HTTP/WS is tight on internal RAM).
  NimBLEProvisionerConfig ble_cfg;
  ble_cfg.scan_interval_ms = 320;
  ble_cfg.scan_window_ms = 30;  // ~9% duty
  g_ble = std::make_shared<NimBLEProvisioner>(ble_cfg);

  // Smaller ad buffer and faster POST interval to keep heap stable.
  BLESignalKGatewayConfig gw_cfg;
  gw_cfg.max_pending_ads = 50;
  gw_cfg.post_interval_ms = 3000;
  g_gateway =
      std::make_shared<BLESignalKGateway>(g_ble, app->get_ws_client(), gw_cfg);
  g_gateway->start();

  event_loop()->onRepeat(5000, []() {
    ESP_LOGI(
        "GW",
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
