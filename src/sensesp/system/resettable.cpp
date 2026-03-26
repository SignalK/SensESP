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
  auto copy = reset_list;
  while (!copy.empty()) {
    copy.top()->reset();
    copy.pop();
  }
}

}  // namespace sensesp
