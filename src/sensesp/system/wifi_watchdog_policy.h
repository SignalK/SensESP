#ifndef SENSESP_SRC_SENSESP_SYSTEM_WIFI_WATCHDOG_POLICY_H_
#define SENSESP_SRC_SENSESP_SYSTEM_WIFI_WATCHDOG_POLICY_H_

#include <cstdint>

#include "sensesp/net/wifi_state.h"

namespace sensesp {

/**
 * @brief Link state as seen by the WiFi watchdog.
 *
 * kNeutral covers states that are neither an outage nor a working link:
 * the soft-AP and the captive portal. The watchdog must not restart a
 * device that is waiting for the user to enter credentials.
 */
enum class WiFiWatchdogLink { kUp, kDown, kNeutral };

/**
 * @brief Map a NetworkState (WiFiState) to the watchdog's view of the link.
 *
 * Only a station or Ethernet connection with a routable IP counts as up.
 * The soft-AP starting does not: on an AP+STA device it starts before the
 * station has ever connected, so treating it as up would arm the watchdog
 * without a real connection.
 */
inline WiFiWatchdogLink wifi_watchdog_link_from_state(WiFiState state) {
  switch (state) {
    case WiFiState::kWifiConnectedToAP:
      return WiFiWatchdogLink::kUp;
    case WiFiState::kWifiNoAP:
    case WiFiState::kWifiDisconnected:
      return WiFiWatchdogLink::kDown;
    case WiFiState::kWifiAPModeActivated:
    case WiFiState::kWifiManagerActivated:
      return WiFiWatchdogLink::kNeutral;
  }
  return WiFiWatchdogLink::kNeutral;
}

/**
 * @brief Pure decision logic for the WiFi watchdog.
 *
 * Dependency-free (no Arduino) so it can be unit-tested on the host.
 *
 * The policy arms on the first link-up after boot. Until then it never
 * requests a restart, so a device that boots without a reachable access
 * point keeps running instead of restarting on every timeout. Once armed, a
 * restart is due when the link has been down continuously for at least the
 * timeout. Repeated down reports keep the original outage start.
 *
 * Times are unsigned milliseconds from a free-running 32-bit clock such as
 * millis(). Elapsed time is computed with modular subtraction so a wrap of the
 * clock during an outage does not fire early or late.
 */
class WiFiWatchdogPolicy {
 public:
  void set_link_state(WiFiWatchdogLink link, uint32_t now_ms) {
    switch (link) {
      case WiFiWatchdogLink::kUp:
        armed_ = true;
        down_ = false;
        break;
      case WiFiWatchdogLink::kDown:
        if (!down_) {
          down_ = true;
          down_since_ms_ = now_ms;
        }
        break;
      case WiFiWatchdogLink::kNeutral:
        down_ = false;
        break;
    }
  }

  bool restart_due(uint32_t now_ms, uint32_t timeout_ms) const {
    if (timeout_ms == 0 || !armed_ || !down_) {
      return false;
    }
    return static_cast<uint32_t>(now_ms - down_since_ms_) >= timeout_ms;
  }

 private:
  bool armed_ = false;
  bool down_ = false;
  uint32_t down_since_ms_ = 0;
};

}  // namespace sensesp

#endif  // SENSESP_SRC_SENSESP_SYSTEM_WIFI_WATCHDOG_POLICY_H_
