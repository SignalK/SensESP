#include "enableable.h"

#include "sensesp.h"

std::priority_queue<Enableable*, std::vector<Enableable*>, EnableableCompare> Enableable::enable_list_;

Enableable::Enableable(int priority) : priority_{priority} {
  Enableable::enable_list_.push(this);
}

void Enableable::enable_all() {
  debugI("Enabling all required sensors and transforms");
  while (!enable_list_.empty()) {
    auto& obj = *enable_list_.top();
    obj.enable();
    enable_list_.pop();
  }  // while
}
