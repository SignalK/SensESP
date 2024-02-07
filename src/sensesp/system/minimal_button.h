#ifndef SENSESP_SRC_SENSESP_SYSTEM_MINIMAL_BUTTON_H_
#define SENSESP_SRC_SENSESP_SYSTEM_MINIMAL_BUTTON_H_

#include "AceButton.h"
#include "elapsedMillis.h"
#include "sensesp.h"
#include "sensesp/system/base_button.h"
#include "sensesp/system/configurable.h"
#include "sensesp_base_app.h"

namespace sensesp {

using namespace ace_button;

/**
 * @brief Minimal Button Handler
 *
 * This is a minimal button handler that implements restart on short press and
 * factory reset on a very long press.
 *
 * This class may be extended to implement more complex button handlers.
 */
class MinimalButtonHandler : public BaseButtonHandler {
 public:
  MinimalButtonHandler(int pin, String config_path = "")
      : BaseButtonHandler(pin, config_path) {}

  virtual void handleEvent(AceButton* button, uint8_t event_type,
                           uint8_t button_state) override {
    debugD("Button event: %d", event_type);
    switch (event_type) {
      case AceButton::kEventPressed:
        time_since_press_event = 0;
        debugD("Press");
        break;
      case AceButton::kEventLongReleased:
        debugD("Long release, duration: %d", time_since_press_event);
        if (time_since_press_event > 5000) {
          this->handle_very_long_press();
        } else if (time_since_press_event > 1000) {
          this->handle_long_press();
        }
        break;
      case AceButton::kEventReleased:
        this->handle_button_press();
        break;
      default:
        break;
    }
  }

 protected:
  elapsedMillis time_since_press_event;
  /**
   * @brief Handle a brief button press (less than one second).
   *
   */
  virtual void handle_button_press() {
    debugD("Short release, duration: %d", time_since_press_event);
    debugD("Restarting");
    ESP.restart();
  }
  /**
   * @brief Handle a long button press (over 1 second).
   *
   */
  virtual void handle_long_press() {
    handle_button_press();
  }
  /**
   * @brief Handle a very long button press (over 5 seconds).
   *
   */
  virtual void handle_very_long_press() {
    debugD("Performing a factory reset");
    SensESPBaseApp::get()->reset();
  }
};


}  // namespace sensesp

#endif  // SENSESP_SRC_SENSESP_SYSTEM_BASE_BUTTON_H_
