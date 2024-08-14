#include "observable.h"

namespace sensesp {

Observable::Observable(Observable&& other) : observers{other.observers} {}

void Observable::notify() {
  for (auto o : observers) {
    o();
  }
}

void Observable::attach(std::function<void()> observer) {
  // First iterate to the last element
  auto before_end = observers.before_begin();
  for (auto& _ : observers) {
    ++before_end;
  }
  // Then insert the new observer
  observers.insert_after(before_end, observer);
}

}  // namespace sensesp
