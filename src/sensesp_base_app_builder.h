#ifndef _sensesp_base_app_builder_H_
#define _sensesp_base_app_builder_H_

#include "sensesp_base_app.h"

namespace sensesp {

class SensESPBaseAppBuilder {
 protected:
  String hostname_ = "SensESP";

 public:
  virtual SensESPBaseAppBuilder* set_hostname(String hostname) = 0;
  virtual SensESPBaseApp* get_app() = 0;
};

}  // namespace sensesp

#endif
