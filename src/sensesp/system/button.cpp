#include "sensesp/system/button.h"

#include "sensesp.h"
#include "sensesp_app.h"

namespace sensesp {

void ButtonHandler::handle_long_press() {
  debugD("Resetting network settings");
  SensESPApp::get()->get_networking()->reset();
}

}  // namespace sensesp
