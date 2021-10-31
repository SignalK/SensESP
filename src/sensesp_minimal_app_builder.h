#ifndef _sensesp_minimal_app_builder_H_
#define _sensesp_minimal_app_builder_H_

#include "sensesp_base_app_builder.h"
#include "sensesp_minimal_app.h"

class SensESPMinimalAppBuilder : public SensESPBaseAppBuilder {
 protected:
  SensESPMinimalApp* app_;

 public:
  SensESPMinimalAppBuilder() { app_ = SensESPMinimalApp::get(); }
  SensESPMinimalAppBuilder* set_hostname(String hostname) override {
    app_->set_preset_hostname(hostname);
    return this;
  }
  SensESPMinimalApp* get_app() override final {
    app_->setup();
    return app_;
  }
};

#endif
