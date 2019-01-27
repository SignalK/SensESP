#include "observable.h"

void Observable::notify() {
  for (auto o: observers) {
    o();
  }
}

void Observable::attach(std::function<void()> observer) {
  observers.push_front(observer);
}
