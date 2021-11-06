#include "observable.h"

namespace sensesp {

Observable::Observable(Observable&& other) : observers{other.observers} {}

void Observable::notify() {
  for (auto o : observers) {
    o();
  }
}

void Observable::attach(std::function<void()> observer) {
  observers.push_front(observer);
}

}  // namespace sensesp
