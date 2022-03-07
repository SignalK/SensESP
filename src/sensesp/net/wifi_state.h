#ifndef SENSESP_NET_WIFI_STATE_H_
#define SENSESP_NET_WIFI_STATE_H_

namespace sensesp {

enum class WiFiState {
  kWifiNoAP = 0,
  kWifiDisconnected,
  kWifiConnectedToAP,
  kWifiManagerActivated
};

// alias WiFiState for backward compatibility
using WifiState = WiFiState;

}  // namespace sensesp

#endif  // SENSESP_NET_WIFI_STATE_H_
