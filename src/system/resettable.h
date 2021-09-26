#ifndef _resettable_H_
#define _resettable_H_

#include <queue>

class ResettableCompare;

/**
 * @brief Automatic calling of the reset() method when the device needs to be
 * reset
 *
 * Classes that implement "Resettable" will have their reset() method
 * called automatically when the device is reset. The optional priority
 * parameter allows the user to control the order in which the objects are
 * reset. The default priority is 0. A higher priority object (priority greater
 * than zero) will be reset before a lower one.
 */
class Resettable {
 public:
  Resettable(int priority = 0);

  virtual void reset() {}

  const int get_reset_priority() const { return priority; }

  void set_priority(int priority) { this->priority = priority; }

  static void reset_all();

  friend class ResettableCompare;

 private:
  int priority;

  static std::priority_queue<Resettable*, std::vector<Resettable*>,
                             ResettableCompare>
      reset_list;
};

class ResettableCompare {
 public:
  bool operator()(Resettable* a, Resettable* b) {
    return a->priority < b->priority;
  }
};

#endif
