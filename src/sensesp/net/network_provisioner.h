#ifndef SENSESP_NET_NETWORK_PROVISIONER_H_
#define SENSESP_NET_NETWORK_PROVISIONER_H_

#include <Arduino.h>
#include <IPAddress.h>

namespace sensesp {

/**
 * @brief Transport-agnostic network provisioner interface.
 *
 * A NetworkProvisioner is responsible for bringing one network interface
 * up (calling WiFi.begin / ETH.begin / …), persisting its configuration,
 * and exposing the runtime IP/MAC information. Each concrete provisioner
 * (WiFiProvisioner, EthernetProvisioner, …) inherits from this base.
 *
 * Network *state* (got IP, lost IP, AP mode, …) is **not** owned by the
 * provisioner. SensESPApp owns a single NetworkStateProducer that
 * subscribes to the unified Arduino-ESP32 Network event bus and emits
 * state transitions for any active interface. Provisioners therefore do
 * not have to coordinate state — they just bring their interface up and
 * the producer notices.
 *
 * Code that needs transport-specific functionality (e.g. WiFi scanning,
 * AP soft-AP IP, ethernet link speed) holds a typed pointer to the
 * concrete provisioner directly. SensESPApp exposes those via
 * get_wifi_provisioner() / get_ethernet_provisioner(), each of which
 * returns nullptr when its transport is not active.
 */
class NetworkProvisioner {
 public:
  virtual ~NetworkProvisioner() = default;

  /// Current local IPv4 address. Returns IPAddress() if no link.
  virtual IPAddress local_ip() const = 0;

  /// Default gateway IPv4 address. Returns IPAddress() if no link.
  virtual IPAddress gateway_ip() const = 0;

  /**
   * @brief Hardware MAC address of the active interface.
   *
   * Format: "AA:BB:CC:DD:EE:FF". For multi-interface chips this returns
   * the MAC of the interface SensESPApp is using to reach the network
   * (e.g. the WiFi STA MAC, or the EMAC MAC).
   */
  virtual String mac_address() const = 0;

  /**
   * @brief True iff the network interface is usable.
   *
   * For STA/Ethernet this means link up + IP obtained. For WiFi in AP
   * mode this also returns true — the device is network-reachable via
   * the soft-AP even though it has no upstream connection. The Signal K
   * websocket client uses this to gate connection attempts; the HTTP
   * server uses it to know when the captive portal should be active.
   */
  virtual bool is_connected() const = 0;
};

}  // namespace sensesp

#endif  // SENSESP_NET_NETWORK_PROVISIONER_H_
