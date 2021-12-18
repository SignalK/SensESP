#include "startable.h"

#include "sensesp.h"

namespace sensesp {

std::priority_queue<Startable*, std::vector<Startable*>, StartableCompare>
    Startable::startable_list_;

Startable::Startable(int priority) : priority_{priority} {
  Startable::startable_list_.push(this);
}

void Startable::start_all() {
  debugI("Starting all required sensors and transforms");
  while (!startable_list_.empty()) {
    auto& obj = *startable_list_.top();
    obj.start();
    startable_list_.pop();
  }  // while
}

}  // namespace sensesp
