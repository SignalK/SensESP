#ifndef SENSESP_NET_WIFI_STATE_H_
#define SENSESP_NET_WIFI_STATE_H_

namespace sensesp {

enum class WifiState {
  kWifiNoAP = 0,
  kWifiDisconnected,
  kWifiConnectedToAP,
  kWifiManagerActivated
};

}  // namespace sensesp

#endif  // SENSESP_NET_WIFI_STATE_H_
