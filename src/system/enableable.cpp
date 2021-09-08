#include "enableable.h"

#include "sensesp_app.h"

std::priority_queue<Enableable*> Enableable::enableList;

Enableable::Enableable(uint8_t priority) : priority{priority} {
  Enableable::enableList.push(this);
}

void Enableable::enable_all() {
  debugD("Enabling all required sensors and transforms");
  while (!enableList.empty()) {
    auto& obj = *enableList.top();
    obj.enable();
    enableList.pop();
  }  // while
}
