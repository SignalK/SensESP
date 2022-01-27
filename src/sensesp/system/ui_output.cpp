#include "ui_output.h"
namespace sensesp {
  std::map<String, UIOutputBase*> uiOutputs;

  UIOutputBase::UIOutputBase(String name) {
    name_ = name;

    uiOutputs[name] = this;
  }
}