#include "system_status_controller.h"

void SystemStatusController::set_input(WifiState new_value,
                                       uint8_t input_channel) {
  // FIXME: If pointers to member functions would be held in an array,
  // this would be a simple array dereferencing
  switch (new_value) {
    case kWifiNoAP:
      this->set_wifi_no_ap();
      break;
    case kWifiDisconnected:
      this->set_wifi_disconnected();
      break;
    case kWifiConnectedToAP:
      this->set_wifi_connected();
      break;
    case kExecutingWifiManager:
      this->set_wifimanager_activated();
      break;
    default:
      this->set_wifi_disconnected();
      break;
  }
}

void SystemStatusController::set_input(WSConnectionState new_value,
                                       uint8_t input_channel) {
  switch (new_value) {
    case kWSDisconnected:
      this->set_ws_disconnected();
      break;
    case kWSConnecting:
      this->set_ws_connecting();
      break;
    case kWSAuthorizing:
      this->set_ws_authorizing();
      break;
    case kWSConnected:
      this->set_ws_connected();
      break;
    default:
      this->set_ws_disconnected();
      break;
  }
}
