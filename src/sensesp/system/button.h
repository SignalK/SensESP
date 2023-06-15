#ifndef SENSESP_SRC_SENSESP_SYSTEM_BUTTON_H_
#define SENSESP_SRC_SENSESP_SYSTEM_BUTTON_H_

#define DEFAULT_BUTTON_PIN 2

#include "AceButton.h"
#include "elapsedMillis.h"
#include "sensesp.h"
#include "sensesp/system/configurable.h"
#include "sensesp/system/minimal_button.h"
#include "sensesp/system/startable.h"

namespace sensesp {

using namespace ace_button;

class ButtonHandler : public MinimalButtonHandler {
 public:
  ButtonHandler(int pin, String config_path = "")
      : MinimalButtonHandler(pin, config_path) {}

 protected:
  virtual void handle_long_press() override;
};

}  // namespace sensesp

#endif  // SENSESP_SRC_SENSESP_SYSTEM_BUTTON_H_
