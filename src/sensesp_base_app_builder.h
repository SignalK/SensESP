#ifndef SENSESP_BASE_APP_BUILDER_H_
#define SENSESP_BASE_APP_BUILDER_H_

#include "sensesp_base_app.h"

namespace sensesp {

/**
 * @brief Abstract base class for other SensESP*AppBuilder classes.
 **/
class SensESPBaseAppBuilder {
 protected:
  String hostname_ = "SensESP";

 public:
  virtual SensESPBaseAppBuilder* set_hostname(String hostname) = 0;
  virtual SensESPBaseApp* get_app() = 0;
};

}  // namespace sensesp

#endif
