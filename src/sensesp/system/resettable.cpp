#include "sensesp.h"

#include "resettable.h"

namespace sensesp {

std::priority_queue<Resettable*, std::vector<Resettable*>, ResettableCompare>
    Resettable::reset_list;

Resettable::Resettable(int priority) : priority(priority) {
  reset_list.push(this);
}

void Resettable::reset_all() {
  ESP_LOGI(__FILENAME__, "Resetting all resettable objects");
  while (!reset_list.empty()) {
    auto& resettable = *reset_list.top();
    resettable.reset();
    reset_list.pop();
  }
}

}  // namespace sensesp
