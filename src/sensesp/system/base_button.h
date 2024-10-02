#ifndef SENSESP_SRC_SENSESP_SYSTEM_BASE_BUTTON_H_
#define SENSESP_SRC_SENSESP_SYSTEM_BASE_BUTTON_H_

#include "sensesp.h"

#include "AceButton.h"
#include "elapsedMillis.h"
#include "sensesp_base_app.h"

namespace sensesp {

using namespace ace_button;

/**
 * @brief Base class for button handlers.
 *
 * Button handlers are used to handle button presses. This is an abstract base
 * class that should be extended to implement a specific button handler.
 */
class BaseButtonHandler : public IEventHandler {
 public:
  BaseButtonHandler(int pin, String config_path = "") {
    button_ = new AceButton(pin);
    pinMode(pin, INPUT_PULLUP);

    ButtonConfig* button_config = button_->getButtonConfig();
    button_config->setIEventHandler(this);
    button_config->setFeature(ButtonConfig::kFeatureLongPress);
    button_config->setFeature(ButtonConfig::kFeatureSuppressAfterLongPress);

    ESP_LOGD(__FILENAME__, "Button handler started");

    event_loop()->onRepeat(
        4, [this]() { this->button_->check(); });
  }

 protected:
  AceButton* button_;
};

}  // namespace sensesp

#endif  // SENSESP_SRC_SENSESP_SYSTEM_BASE_BUTTON_H_
