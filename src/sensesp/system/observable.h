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
  Observable(Observable&& other);

  void notify();
  void attach(std::function<void()> observer);

 private:
  std::forward_list<std::function<void()> > observers_;
};

}  // namespace sensesp

#endif
