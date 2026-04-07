#include "sensesp.h"

#include "ethernet_provisioner.h"

#if defined(CONFIG_IDF_TARGET_ESP32P4)

#include <Network.h>

namespace sensesp {

EthernetProvisioner::EthernetProvisioner(const EthernetConfig& config)
    : config_(config) {
  ESP_LOGI(__FILENAME__,
           "Bringing up Ethernet (PHY type=%d addr=%d MDC=%d MDIO=%d "
           "PWR=%d clk_mode=%d)",
           (int)config_.phy_type, (int)config_.phy_addr, config_.mdc_pin,
           config_.mdio_pin, config_.power_pin, (int)config_.clock_mode);

  // Register an ARDUINO_EVENT_ETH_START listener that applies the
  // SensESP hostname to the ETH interface right after the netif has
  // been created but before DHCP DISCOVER goes out. This is the only
  // guaranteed-correct moment:
  //
  //   - Before ETH.begin():  _esp_netif is still NULL so
  //     ETH.setHostname() returns false and the hostname is dropped
  //     (this is what the previous implementation did, causing the
  //     DHCP client to advertise "espressif" — the Arduino ESP32
  //     variant's compile-time default — instead of the SensESP
  //     hostname).
  //
  //   - After ETH.begin():   the netif exists, but esp_eth_start()
  //     already ran and the DHCP client may have already sent its
  //     DISCOVER, so setting the hostname here would land after the
  //     server has recorded the wrong one.
  //
  //   - In the ETH_START event:  fires synchronously from the
  //     network event loop after the netif is created and before
  //     the DHCP client starts. This is the canonical pattern used
  //     by the Arduino-ESP32 ETH_LAN8720 example.
  //
  // Network.onEvent() callbacks run on a separate FreeRTOS task
  // (the LwIP / system event task), so the capture must be plain
  // value-captured — no reference to `this`.
  String hostname = SensESPBaseApp::get_hostname();
  Network.onEvent(
      [hostname](arduino_event_id_t, arduino_event_info_t) {
        if (!ETH.setHostname(hostname.c_str())) {
          ESP_LOGW("eth_prov",
                   "ETH.setHostname(\"%s\") failed in ETH_START handler",
                   hostname.c_str());
        } else {
          ESP_LOGI("eth_prov",
                   "Ethernet hostname set to \"%s\" before DHCP DISCOVER",
                   hostname.c_str());
        }
      },
      ARDUINO_EVENT_ETH_START);

  // Use the explicit RMII begin() so SensESP doesn't depend on the
  // variant supplying compile-time ETH_PHY_* macros — although on the
  // current Waveshare ESP32-P4 board the variant happens to match.
  bool started = ETH.begin(config_.phy_type, config_.phy_addr, config_.mdc_pin,
                           config_.mdio_pin, config_.power_pin,
                           config_.clock_mode);
  if (!started) {
    ESP_LOGE(__FILENAME__,
             "ETH.begin() failed — check pin mapping and PHY power.");
    return;
  }

  ESP_LOGI(__FILENAME__,
           "Ethernet driver started; waiting for PHY link + DHCP lease.");
}

EthernetProvisioner::~EthernetProvisioner() {
  // Arduino's ETH does not provide a clean shutdown that re-initialises
  // cleanly afterwards, so we deliberately do nothing here. The
  // NetworkStateProducer in SensESPApp owns the event subscriptions.
}

IPAddress EthernetProvisioner::local_ip() const { return ETH.localIP(); }

IPAddress EthernetProvisioner::gateway_ip() const { return ETH.gatewayIP(); }

String EthernetProvisioner::mac_address() const { return ETH.macAddress(); }

bool EthernetProvisioner::is_connected() const {
  // Considered connected once the PHY link is up *and* the IP stack
  // has obtained an address. The Arduino ETH wrapper exposes both
  // through its NetworkInterface base class.
  return ETH.linkUp() && ETH.hasIP();
}

int EthernetProvisioner::link_speed_mbps() const {
  return ETH.linkUp() ? ETH.linkSpeed() : 0;
}

bool EthernetProvisioner::is_full_duplex() const {
  return ETH.linkUp() && ETH.fullDuplex();
}

}  // namespace sensesp

#endif  // CONFIG_IDF_TARGET_ESP32P4
