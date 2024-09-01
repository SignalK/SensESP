#include "observable.h"

namespace sensesp {

Observable::Observable(Observable&& other) : observers_{other.observers_} {}

void Observable::notify() {
  for (auto o : observers_) {
    o();
  }
}

void Observable::attach(std::function<void()> observer) {
  // First iterate to the last element
  auto before_end = observers_.before_begin();
  for (auto& _ : observers_) {
    ++before_end;
  }
  // Then insert the new observer
  observers_.insert_after(before_end, observer);
}

}  // namespace sensesp
