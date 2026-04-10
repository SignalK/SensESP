#ifndef SENSESP_NET_ETHERNET_PROVISIONER_H_
#define SENSESP_NET_ETHERNET_PROVISIONER_H_

// Native RMII Ethernet support.
//
// Currently only ESP32-P4 ships an Arduino-ESP32 ETH.h implementation
// that SensESP can drive without writing a custom EMAC bring-up, so the
// entire EthernetProvisioner class is guarded behind CONFIG_IDF_TARGET_ESP32P4.
// On any other target, including this header compiles to nothing and
// `EthernetConfig` / `EthernetProvisioner` are simply not declared —
// so a user sketch that tries to call
// `builder.set_ethernet(EthernetConfig::…())` on a classic ESP32 will
// get a clear compile error at the use site ("'EthernetConfig' was
// not declared in this scope") rather than a silent runtime failure.
//
// When future ESP32 variants grow a usable Arduino-ESP32 ETH driver,
// the guard can be widened accordingly.

#include <Arduino.h>

#if defined(CONFIG_IDF_TARGET_ESP32P4)

#include <ETH.h>

#include "sensesp/net/network_provisioner.h"
#include "sensesp_base_app.h"

namespace sensesp {

class EthernetProvisioner;

/**
 * @brief Hardware description for an EthernetProvisioner.
 *
 * Selects the PHY model, address, MDC/MDIO pins, reset pin, and clock
 * direction. The Arduino-ESP32 ETH library reads the RMII data pins
 * (TX0/1, RX0/1, TX_EN, CRS_DV, REF_CLK) from compile-time macros in
 * the board variant, so this struct intentionally does not expose them.
 *
 * Use the static factory methods to get a config for a known board:
 *
 *     builder.set_ethernet(EthernetConfig::waveshare_esp32p4_poe());
 */
struct EthernetConfig {
  /// Tag used by SensESPAppBuilder::set_ethernet() (template) to find
  /// the corresponding provisioner implementation type.
  using ProvisionerType = EthernetProvisioner;

  eth_phy_type_t phy_type = ETH_PHY_IP101;
  int32_t phy_addr = 1;
  int mdc_pin = 31;
  int mdio_pin = 52;
  int power_pin = 51;  // PHY reset pin
  eth_clock_mode_t clock_mode = EMAC_CLK_EXT_IN;

  /// Configuration for the Waveshare ESP32-P4-WIFI6-POE-ETH dev board.
  /// PHY: IP101GRI (ETH_PHY_IP101 is an Arduino-ESP32 alias for the
  /// ETH_PHY_TLK110 IDF driver, which IP101GRI is electrically
  /// compatible with). REF_CLK is driven by an external 25 MHz crystal
  /// frequency-doubled by the PHY into GPIO50.
  static EthernetConfig waveshare_esp32p4_poe() {
    EthernetConfig cfg;
    cfg.phy_type = ETH_PHY_IP101;
    cfg.phy_addr = 1;
    cfg.mdc_pin = 31;
    cfg.mdio_pin = 52;
    cfg.power_pin = 51;
    cfg.clock_mode = EMAC_CLK_EXT_IN;
    return cfg;
  }
};

/**
 * @brief NetworkProvisioner for native ESP32-P4 RMII Ethernet.
 *
 * Brings up the ESP32-P4 EMAC + an external RMII PHY via the Arduino
 * ETH library and runs DHCP. Network *state* (got IP, lost IP, …) is
 * not owned here — SensESPApp's NetworkStateProducer subscribes to the
 * unified Arduino-ESP32 event bus and notices ethernet state changes
 * automatically.
 *
 * No WiFi soft-AP, no captive portal — Ethernet is assumed to provide
 * upstream connectivity unconditionally.
 */
class EthernetProvisioner : public NetworkProvisioner {
 public:
  explicit EthernetProvisioner(const EthernetConfig& config);
  ~EthernetProvisioner() override;

  // -- NetworkProvisioner --
  IPAddress local_ip() const override;
  IPAddress gateway_ip() const override;
  String mac_address() const override;
  bool is_connected() const override;

  // -- Ethernet-specific --
  /// Link speed in megabits per second (10 or 100), 0 if no link.
  int link_speed_mbps() const;

  /// True iff link is currently full-duplex.
  bool is_full_duplex() const;

 private:
  EthernetConfig config_;
};

}  // namespace sensesp

#endif  // CONFIG_IDF_TARGET_ESP32P4

#endif  // SENSESP_NET_ETHERNET_PROVISIONER_H_
