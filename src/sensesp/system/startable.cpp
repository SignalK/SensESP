#include "startable.h"

#include "sensesp.h"

namespace sensesp {

std::priority_queue<Startable*, std::vector<Startable*>, StartableCompare>
    Startable::startable_queue_;

Startable::Startable(int priority) : priority_{priority} {
  Startable::startable_queue_.push(this);
}

void Startable::start_all() {
  debugI("Starting all required sensors and transforms");
  while (!startable_queue_.empty()) {
    auto& obj = *startable_queue_.top();
    obj.start();
    startable_queue_.pop();
  }  // while
}

}  // namespace sensesp
