#ifndef SENSESP_SRC_SENSESP_SYSTEM_SYSTEM_STATUS_LED_H_
#define SENSESP_SRC_SENSESP_SYSTEM_SYSTEM_STATUS_LED_H_

#include <memory>

#include "lambda_consumer.h"
#include "led_blinker.h"
#include "sensesp/controllers/system_status_controller.h"

namespace sensesp {

/**
 * @brief Consumes the networking and websocket states and delta counts
 * and updates the device LED accordingly. Inherit this class and override
 * the methods to customize the behavior.
 */
class SystemStatusLed {
 protected:
  std::unique_ptr<PatternBlinker> blinker_;

  virtual void set_wifi_no_ap();
  virtual void set_wifi_disconnected();
  virtual void set_wifimanager_activated();

  virtual void set_ws_disconnected();
  virtual void set_ws_authorizing();
  virtual void set_ws_connecting();
  virtual void set_ws_connected();

  LambdaConsumer<SystemStatus> system_status_consumer_{
      [this](SystemStatus status) {
        switch (status) {
          case SystemStatus::kWifiNoAP:
            this->set_wifi_no_ap();
            break;
          case SystemStatus::kWifiDisconnected:
            this->set_wifi_disconnected();
            break;
          case SystemStatus::kWifiManagerActivated:
            this->set_wifimanager_activated();
            break;
          case SystemStatus::kSKWSDisconnected:
            this->set_ws_disconnected();
            break;
          case SystemStatus::kSKWSConnecting:
            this->set_ws_connecting();
            break;
          case SystemStatus::kSKWSAuthorizing:
            this->set_ws_authorizing();
            break;
          case SystemStatus::kSKWSConnected:
            this->set_ws_connected();
            break;
        }
      }};

  LambdaConsumer<int> delta_tx_count_consumer_{
      [this](int) { blinker_->blip(); }};

 public:
  SystemStatusLed(int pin);

  ValueConsumer<SystemStatus>& get_system_status_consumer() {
    return system_status_consumer_;
  }
  ValueConsumer<int>& get_delta_tx_count_consumer() {
    return delta_tx_count_consumer_;
  }
};

}  // namespace sensesp

#endif
