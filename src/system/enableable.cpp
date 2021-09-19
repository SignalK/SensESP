#include "enableable.h"

#include "sensesp_app.h"

std::priority_queue<Enableable*, std::vector<Enableable*>, EnableableCompare> Enableable::enable_list;

Enableable::Enableable(int priority) : priority{priority} {
  Enableable::enable_list.push(this);
}

void Enableable::enable_all() {
  debugD("Enabling all required sensors and transforms");
  while (!enable_list.empty()) {
    auto& obj = *enable_list.top();
    obj.enable();
    enable_list.pop();
  }  // while
}
