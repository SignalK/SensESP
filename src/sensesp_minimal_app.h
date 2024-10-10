#ifndef SENSESP_MINIMAL_APP_H_
#define SENSESP_MINIMAL_APP_H_

#include <memory>

#include "sensesp_base_app.h"

namespace sensesp {

class SensESPMinimalApp : public SensESPBaseApp {
 public:
  SensESPMinimalApp(SensESPMinimalApp &other) = delete;
  void operator=(SensESPMinimalApp &other) = delete;

  /**
   * @brief Get the singleton instance of the SensESPMinimalApp
   */
  static const std::shared_ptr<SensESPMinimalApp>& get() {
    if (instance_ == nullptr) {
      instance_ = std::shared_ptr<SensESPMinimalApp>(new SensESPMinimalApp());
    }
    return std::static_pointer_cast<SensESPMinimalApp>(instance_);
  }

 protected:
  SensESPMinimalApp() : SensESPBaseApp() {}

  friend class SensESPMinimalAppBuilder;
};

}  // namespace sensesp

#endif
