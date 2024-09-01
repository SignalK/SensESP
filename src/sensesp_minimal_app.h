#ifndef SENSESP_MINIMAL_APP_H_
#define SENSESP_MINIMAL_APP_H_

#include "sensesp_base_app.h"

namespace sensesp {

class SensESPMinimalApp : public SensESPBaseApp {
 public:
  SensESPMinimalApp(SensESPMinimalApp &other) = delete;
  void operator=(SensESPMinimalApp &other) = delete;

  /**
   * @brief Get the singleton instance of the SensESPMinimalApp
   */
  static SensESPMinimalApp *get();

 protected:
  SensESPMinimalApp() : SensESPBaseApp() {}

  friend class SensESPMinimalAppBuilder;
};

}  // namespace sensesp

#endif
