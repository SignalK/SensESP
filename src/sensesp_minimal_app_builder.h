#ifndef SENSESP_MINIMAL_APP_BUILDER_H
#define SENSESP_MINIMAL_APP_BUILDER_H

#include "sensesp_base_app_builder.h"
#include "sensesp_minimal_app.h"

namespace sensesp {

class SensESPMinimalAppBuilder : public SensESPBaseAppBuilder {
 protected:
  std::shared_ptr<SensESPMinimalApp> app_;

 public:
  SensESPMinimalAppBuilder() { app_ = SensESPMinimalApp::get(); }
  SensESPMinimalAppBuilder* set_hostname(String hostname) override {
    app_->set_hostname(hostname);
    return this;
  }
  std::shared_ptr<SensESPMinimalApp> get_app() {
    app_->setup();
    return app_;
  }
};

}  // namespace sensesp
#endif
