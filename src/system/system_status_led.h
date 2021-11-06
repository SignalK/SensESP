#ifndef _LED_CONTROLLER_H_
#define _LED_CONTROLLER_H_

#include "controllers/system_status_controller.h"
#include "led_blinker.h"
#include "system/lambda_consumer.h"

namespace sensesp {

/**
 * @brief Consumes the networking and websocket states and delta counts
 * and updates the device LED accordingly. Inherit this class and override
 * the methods to customize the behavior.
 */
class SystemStatusLed : public ValueConsumer<SystemStatus>,
                        public ValueConsumer<int> {
 protected:
  PatternBlinker* blinker_;

  virtual void set_wifi_no_ap();
  virtual void set_wifi_disconnected();
  virtual void set_wifimanager_activated();

  virtual void set_ws_disconnected();
  virtual void set_ws_authorizing();
  virtual void set_ws_connecting();
  virtual void set_ws_connected();

 public:
  SystemStatusLed(int pin);

  virtual void set_input(SystemStatus new_value,
                         uint8_t input_channel = 0) override;
  virtual void set_input(int new_value, uint8_t input_channel = 0) override;
};

}  // namespace sensesp

#endif
