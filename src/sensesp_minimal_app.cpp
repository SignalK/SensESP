#include "sensesp_minimal_app.h"

namespace sensesp {

SensESPMinimalApp* SensESPMinimalApp::get() {
  if (instance_ == nullptr) {
    instance_ = new SensESPMinimalApp();
  }
  return static_cast<SensESPMinimalApp*>(instance_);
}

}  // namespace sensesp
