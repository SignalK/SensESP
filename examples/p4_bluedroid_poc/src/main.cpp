/**
 * @file main.cpp
 * @brief SensESP P4 + Bluedroid PoC — stage 1 baseline firmware.
 *
 * Minimal SensESP app that exercises the new `framework = espidf,
 * arduino` build mode extended to ESP32-P4. The project's
 * sdkconfig.defaults selects Bluedroid as the BT host stack (instead
 * of NimBLE, which is what the Arduino-ESP32 prebuilt libs ship on
 * P4), so this firmware links Bluedroid in — but does not yet call
 * into it. Stage 2 will add the actual Bluedroid scanner on top of
 * this baseline.
 *
 * What this firmware exercises:
 *   - SensESP + transport-agnostic networking + EthernetProvisioner
 *     on the Waveshare ESP32-P4-WIFI6-POE-ETH board
 *   - The `[env:espidf_esp32p4]` build path and its sdkconfig
 *   - Hostname set before DHCP DISCOVER via the ETH_START handler
 *   - mDNS, HTTP server, OTA, and the Signal K websocket client
 *
 * Verification checklist on the host (with a DHCP server on the
 * same LAN):
 *   1. Device boots, serial prints the usual IDF init banner
 *   2. "Bringing up Ethernet" line appears
 *   3. PHY link comes up when the cable is plugged in
 *   4. DHCP lease obtained; check the ARP table
 *   5. http://signalk-bluedroid-poc.local/ serves the SensESP web UI
 *   6. OTA flashing works via espota.py on port 3232
 *   7. Heartbeat log line every 5 seconds
 */

#include "sensesp/net/ethernet_provisioner.h"
#include "sensesp_app_builder.h"

using namespace sensesp;

void setup() {
  SetupLogging(ESP_LOG_INFO);

  SensESPAppBuilder builder;
  builder.set_hostname(GATEWAY_HOSTNAME)
      ->set_ethernet(EthernetConfig::waveshare_esp32p4_poe())
      ->enable_ota("bluedroid-poc-ota")
      ->get_app();

  event_loop()->onRepeat(5000, []() {
    ESP_LOGI("POC", "alive — uptime=%lus heap=%u",
             (unsigned long)(millis() / 1000),
             (unsigned)ESP.getFreeHeap());
  });
}

void loop() { event_loop()->tick(); }
