#ifndef SENSESP_SYSTEM_OBSERVABLE_H
#define SENSESP_SYSTEM_OBSERVABLE_H

#include <forward_list>
#include <functional>
#include <vector>

namespace sensesp {

/**
 * @brief A base class which allow observers
 * to attach callbacks to themselves. The callbacks
 * will be called when the observable needs to
 * notify the observers about some state change.
 */
class Observable {
 public:
  Observable() {}

  /// Move constructor
  Observable(Observable&& other) : observers_{other.observers_} {}

  void notify() {
    for (auto o : observers_) {
      o();
    }
  }

  void attach(std::function<void()> observer) {
    // First iterate to the last element
    auto before_end = observers_.before_begin();
    for (auto& _ : observers_) {
      ++before_end;
    }
    // Then insert the new observer
    observers_.insert_after(before_end, observer);
  }

 private:
  std::forward_list<std::function<void()> > observers_;
};

}  // namespace sensesp

#endif
