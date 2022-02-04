#include "ui_output.h"
namespace sensesp {
std::map<String, UIOutputBase*> ui_outputs;

UIOutputBase::UIOutputBase(String name, String group, int order) {
  name_ = name;
  group_ = group;
  order_ = order;
  ui_outputs[name] = this;
}
}  // namespace sensesp