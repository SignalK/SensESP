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
  struct ObserverEntry {
    int id;
    std::function<void()> callback;
  };

  Observable() {}

  /// Move constructor
  Observable(Observable&& other)
      : observers_{std::move(other.observers_)},
        next_id_{other.next_id_} {}

  void notify() {
    for (const ObserverEntry& entry : observers_) {
      entry.callback();
    }
  }

  /**
   * @brief Attach an observer callback.
   * @return An ID that can be passed to detach() to remove the observer.
   */
  int attach(std::function<void()> observer) {
    int id = next_id_++;
    // Iterate to the last element
    auto before_end = observers_.before_begin();
    for (auto& _ : observers_) {
      ++before_end;
    }
    // Insert the new observer
    observers_.insert_after(before_end, ObserverEntry{id, observer});
    return id;
  }

  /**
   * @brief Remove a previously attached observer by its ID.
   */
  void detach(int id) {
    observers_.remove_if(
        [id](const ObserverEntry& entry) { return entry.id == id; });
  }

 private:
  std::forward_list<ObserverEntry> observers_;
  int next_id_ = 0;
};

}  // namespace sensesp

#endif
