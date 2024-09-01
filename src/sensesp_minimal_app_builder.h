#ifndef SENSESP_MINIMAL_APP_BUILDER_H
#define SENSESP_MINIMAL_APP_BUILDER_H

#include "sensesp_base_app_builder.h"
#include "sensesp_minimal_app.h"

namespace sensesp {

class SensESPMinimalAppBuilder : public SensESPBaseAppBuilder {
 protected:
  SensESPMinimalApp* app_;

 public:
  SensESPMinimalAppBuilder() { app_ = SensESPMinimalApp::get(); }
  SensESPMinimalAppBuilder* set_hostname(String hostname) override {
    app_->set_hostname(hostname);
    return this;
  }
  SensESPMinimalApp* get_app() override final {
    app_->setup();
    return app_;
  }
};

}  // namespace sensesp
#endif
