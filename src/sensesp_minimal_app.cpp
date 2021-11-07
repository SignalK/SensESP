#include "sensesp_minimal_app.h"

namespace sensesp {

SensESPMinimalApp* SensESPMinimalApp::get() {
  if (instance_ == nullptr) {
    instance_ = new SensESPMinimalApp();
  }
  return (SensESPMinimalApp*)instance_;
}

}  // namespace sensesp
