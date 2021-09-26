#ifndef _sensesp_base_app_builder_H_
#define _sensesp_base_app_builder_H_

#include "sensesp_base_app.h"

class SensESPBaseAppBuilder {
 private:
  String hostname_ = "SensESP";

 protected:
  SensESPBaseApp* app_;

 public:
  SensESPBaseAppBuilder() {
    app_ = SensESPBaseApp::get();
  }
  SensESPBaseAppBuilder* set_hostname(String hostname) {
    app_->set_preset_hostname(hostname);
    return this;
  }
  SensESPBaseApp* get_app() {
    app_->setup();
    return app_;
  }
};

#endif
