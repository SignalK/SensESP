#ifndef _sensesp_minimal_app_builder_H_
#define _sensesp_minimal_app_builder_H_

#include "sensesp_minimal_app.h"

class SensESPMinimalAppBuilder {
 private:
  String hostname_ = "SensESP";

 protected:
  SensESPMinimalApp* app_;

 public:
  SensESPMinimalAppBuilder() {
    app_ = SensESPMinimalApp::get();
  }
  SensESPMinimalAppBuilder* set_hostname(String hostname) {
    app_->set_preset_hostname(hostname);
    return this;
  }
  SensESPMinimalApp* get_app() {
    app_->setup();
    return app_;
  }
};

#endif
