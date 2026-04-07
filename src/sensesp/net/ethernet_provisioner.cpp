#include "sensesp.h"

#include "ethernet_provisioner.h"

#if defined(CONFIG_IDF_TARGET_ESP32P4)

namespace sensesp {

EthernetProvisioner::EthernetProvisioner(const EthernetConfig& config)
    : config_(config) {
  ESP_LOGI(__FILENAME__,
           "Bringing up Ethernet (PHY type=%d addr=%d MDC=%d MDIO=%d "
           "PWR=%d clk_mode=%d)",
           (int)config_.phy_type, (int)config_.phy_addr, config_.mdc_pin,
           config_.mdio_pin, config_.power_pin, (int)config_.clock_mode);

  // Set the hostname before bringing up the link so DHCP REQUESTs carry
  // it. SensESP stores the hostname on the SensESPBaseApp singleton.
  String hostname = SensESPBaseApp::get_hostname();
  ETH.setHostname(hostname.c_str());

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
