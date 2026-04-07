#ifndef SENSESP_NET_NETWORK_STATE_H_
#define SENSESP_NET_NETWORK_STATE_H_

#include <cstddef>

#include "sensesp/net/wifi_state.h"
#include "sensesp/system/valueproducer.h"

namespace sensesp {

/**
 * @brief Transport-agnostic network state.
 *
 * Replaces the WiFi-specific WiFiState enum with values that any
 * transport (WiFi, Ethernet, ESP-Hosted, future PPP/cellular, …) can
 * produce. The legacy WiFiState enum is kept as a strong alias of
 * NetworkState in wifi_state.h, and existing user code that uses the
 * old kWifi* member names continues to compile unchanged.
 *
 * Semantic mapping (new neutral name → existing kWifi* alias):
 *   kNoConnection   ↔ kWifiNoAP
 *   kDisconnected   ↔ kWifiDisconnected
 *   kConnected      ↔ kWifiConnectedToAP   (interface has a routable IP)
 *   kAPMode         ↔ kWifiAPModeActivated (WiFi-only soft-AP fallback)
 *   kProvisioning   ↔ kWifiManagerActivated (interactive provisioner)
 *
 * Non-WiFi provisioners must never emit kAPMode or kProvisioning.
 */
using NetworkState = WiFiState;

/// Neutral aliases that new transport-agnostic code can use instead of
/// the historical kWifi* names. Both compile to the same enum values.
inline constexpr NetworkState kNetworkNoConnection = NetworkState::kWifiNoAP;
inline constexpr NetworkState kNetworkDisconnected = NetworkState::kWifiDisconnected;
inline constexpr NetworkState kNetworkConnected = NetworkState::kWifiConnectedToAP;
inline constexpr NetworkState kNetworkAPMode = NetworkState::kWifiAPModeActivated;
inline constexpr NetworkState kNetworkProvisioning =
    NetworkState::kWifiManagerActivated;

/**
 * @brief Unified producer that emits NetworkState transitions.
 *
 * Owned by SensESPApp. Subscribes to the Arduino-ESP32 unified network
 * event bus (Network.onEvent) and listens for both WiFi and Ethernet
 * IP/disconnect events. Emits a NetworkState whenever any interface's
 * IP-layer state changes.
 *
 * Replaces the legacy WiFiStateProducer that only listened to WIFI_*
 * events. The header keeps only the declaration so files that include
 * it (e.g. system_status_controller.h) do not pull in Network.h.
 */
class NetworkStateProducer : public ValueProducer<NetworkState> {
 public:
  NetworkStateProducer();
  ~NetworkStateProducer();

  NetworkStateProducer(const NetworkStateProducer&) = delete;
  NetworkStateProducer& operator=(const NetworkStateProducer&) = delete;

 private:
  // Opaque Arduino-ESP32 network_event_handle_t (= size_t). Stored as
  // size_t in the header so we don't need to pull Network.h here.
  std::size_t wifi_got_ip_handle_ = 0;
  std::size_t wifi_disconnected_handle_ = 0;
  std::size_t wifi_ap_start_handle_ = 0;
  std::size_t wifi_ap_stop_handle_ = 0;
  std::size_t eth_got_ip_handle_ = 0;
  std::size_t eth_lost_ip_handle_ = 0;
  std::size_t eth_disconnected_handle_ = 0;
};

}  // namespace sensesp

#endif  // SENSESP_NET_NETWORK_STATE_H_
