#ifndef SENSESP_SRC_SENSESP_SYSTEM_BASE_BUTTON_H_
#define SENSESP_SRC_SENSESP_SYSTEM_BASE_BUTTON_H_

#include "AceButton.h"
#include "elapsedMillis.h"
#include "sensesp.h"
#include "sensesp/system/configurable.h"
#include "sensesp/system/startable.h"
#include "sensesp_base_app.h"

namespace sensesp {

using namespace ace_button;

/**
 * @brief Base class for button handlers.
 *
 * Button handlers are used to handle button presses. This is an abstract base
 * class that should be extended to implement a specific button handler.
 */
class BaseButtonHandler : public Configurable, public Startable, public IEventHandler {
 public:
  BaseButtonHandler(int pin, String config_path = "")
      : Configurable{config_path}, Startable(20) {
    button_ = new AceButton(pin);
    pinMode(pin, INPUT_PULLUP);
  }

  virtual void start() override {
    ButtonConfig* button_config = button_->getButtonConfig();
    button_config->setIEventHandler(this);
    button_config->setFeature(ButtonConfig::kFeatureLongPress);
    button_config->setFeature(ButtonConfig::kFeatureSuppressAfterLongPress);

    debugD("Button handler started");

    ReactESP::app->onRepeat(4, [this]() { this->button_->check(); });
  }

 protected:
  AceButton* button_;
};


}  // namespace sensesp

#endif  // SENSESP_SRC_SENSESP_SYSTEM_BASE_BUTTON_H_
