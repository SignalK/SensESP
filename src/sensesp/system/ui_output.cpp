#include "ui_output.h"
namespace sensesp {
std::map<String, UIOutputBase*> ui_outputs;

UIOutputBase::UIOutputBase(String name) {
  name_ = name;

  ui_outputs[name] = this;
}
}  // namespace sensesp