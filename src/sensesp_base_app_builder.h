#ifndef _sensesp_base_app_builder_H_
#define _sensesp_base_app_builder_H_

#include "sensesp_base_app.h"

class SensESPBaseAppBuilder {
 private:
  String hostname_ = "SensESP";

 protected:
  SensESPBaseApp* app;

 public:
  SensESPBaseAppBuilder() {
    app = new SensESPBaseApp(true);
  }
  SensESPBaseAppBuilder* set_hostname(String hostname) {
    app->set_preset_hostname(hostname);
    return this;
  }
};

#endif
