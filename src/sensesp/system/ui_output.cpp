#include "ui_output.h"

namespace sensesp {

std::map<String, UIOutputBase*> UIOutputBase::ui_outputs_;

UIOutputBase::UIOutputBase(String name) {
  name_ = name;

  ui_outputs_[name] = this;
}
}  // namespace sensesp
