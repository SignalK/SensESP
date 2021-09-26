#include "resettable.h"

#include "sensesp.h"

std::priority_queue<Resettable*, std::vector<Resettable*>, ResettableCompare>
    Resettable::reset_list;

Resettable::Resettable(int priority) : priority(priority) {
  reset_list.push(this);
}

void Resettable::reset_all() {
  debugI("Resetting all resettable objects");
  while (!reset_list.empty()) {
    auto& resettable = *reset_list.top();
    resettable.reset();
    reset_list.pop();
  }
}
