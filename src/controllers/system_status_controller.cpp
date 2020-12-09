#include "system_status_controller.h"

void SystemStatusController::set_input(WifiState new_value,
                                       uint8_t input_channel) {
  // FIXME: If pointers to member functions would be held in an array,
  // this would be a simple array dereferencing
  switch (new_value) {
    case WifiState::kWifiNoAP:
      this->emit(SystemStatus::kWifiNoAP);
      break;
    case WifiState::kWifiDisconnected:
      this->emit(SystemStatus::kWifiDisconnected);
      break;
    case WifiState::kWifiConnectedToAP:
      this->emit(SystemStatus::kWSDisconnected);
      break;
    case WifiState::kWifiManagerActivated:
      this->emit(SystemStatus::kWifiManagerActivated);
      break;
  }
}

void SystemStatusController::set_input(WSConnectionState new_value,
                                       uint8_t input_channel) {
  switch (new_value) {
    case WSConnectionState::kWSDisconnected:
      this->emit(SystemStatus::kWSDisconnected);
      break;
    case WSConnectionState::kWSConnecting:
      this->emit(SystemStatus::kWSConnecting);
      break;
    case WSConnectionState::kWSAuthorizing:
      this->emit(SystemStatus::kWSAuthorizing);
      break;
    case WSConnectionState::kWSConnected:
      this->emit(SystemStatus::kWSConnected);
      break;
  }
}
