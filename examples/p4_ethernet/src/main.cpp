/**
 * @file main.cpp
 * @brief Minimal SensESP app running on native Ethernet on the
 *        Waveshare ESP32-P4-WIFI6-POE-ETH board.
 *
 * Uses the transport-agnostic NetworkProvisioner abstraction and the
 * Waveshare-tuned EthernetProvisioner config. WiFi is disabled
 * entirely — the device runs over native ESP32-P4 RMII Ethernet to
 * the onboard IP101GRI PHY and can be PoE-powered via the RJ45
 * magjack.
 *
 * Verification checklist on the host (with a DHCP server on the
 * same LAN):
 *   1. Device boots, serial prints "Bringing up Ethernet"
 *   2. PHY link comes up when a cable is plugged in
 *   3. DHCP lease obtained; check the ARP table on the host
 *   4. http://<leased-ip>/ serves the SensESP web UI
 *   5. Configure the Signal K server address in the web UI
 *   6. SK connection status on the status page flips to "Connected"
 */

#include "sensesp/net/ethernet_provisioner.h"
#include "sensesp_app_builder.h"

using namespace sensesp;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("[p4_eth] boot");

  SensESPAppBuilder builder;
  auto sensesp_app = builder.set_hostname("sensesp-p4-eth")
                         ->set_ethernet(EthernetConfig::waveshare_esp32p4_poe())
                         ->enable_uptime_sensor()
                         ->enable_free_mem_sensor()
                         ->enable_ip_address_sensor()
                         ->get_app();

  Serial.println("[p4_eth] SensESPApp constructed");
}

void loop() { event_loop()->tick(); }
