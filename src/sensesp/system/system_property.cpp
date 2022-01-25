#include "system_property.h"
namespace sensesp {
  std::map<String, SystemPropertyBase*> systemProperties;

  SystemPropertyBase::SystemPropertyBase(String name) {
    name_ = name;

    systemProperties[name] = this;
  }
}