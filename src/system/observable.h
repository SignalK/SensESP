#ifndef _observable_H_
#define _observable_H_

#include <forward_list>
#include <functional>
#include <vector>

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
  std::forward_list<std::function<void()> > observers;
};

#endif
