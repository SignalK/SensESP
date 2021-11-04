#include "sensesp_minimal_app.h"

SensESPMinimalApp* SensESPMinimalApp::get() {
  if (instance_ == nullptr) {
    instance_ = new SensESPMinimalApp();
  }
  return (SensESPMinimalApp*)instance_;
}
