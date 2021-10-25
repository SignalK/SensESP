#ifndef _startable_H_
#define _startable_H_

#include <queue>

class StartableCompare;

/**
 * @brief Automatic calling of the start() method at startup
 * 
 * Classes that implement "Startable" will have their start() method
 * called automatically at startup when the SensESP app itself
 * is started.  The optional priority allows for certain classes
 * to be initialized first. The default priority is zero.
 * A higher priority (i.e. greater than zero) will be started before a
 * lower one (i.e. less than zero).
 */
class Startable {
 public:
  Startable(int priority = 0);

  /**
   * Called during the initialization process.  Override this method
   * to add runtime initialization code to your class
   */
  virtual void start() {}

  const int get_start_priority() { return priority_; }

  void set_priority(int priority) { this->priority_ = priority; }

  /**
   * Called by the SensESP framework to initialize all of the objects
   * marked with this class. They will be initialized in priorty
   * order. If you want to see see the name of each sensor and transport
   * in the serial monitor as each one is enabled, add the following
   * to your project's platformio.ini file:
   *
   * build_unflags = -fno-rtti
   */
  static void start_all();

  friend class StartableCompare;
 private:
  int priority_;

  static std::priority_queue<Startable*, std::vector<Startable*>, StartableCompare> startable_list_;
};

class StartableCompare {
  public:
    bool operator()(const Startable* lhs, const Startable* rhs) const {
      return lhs->priority_ < rhs->priority_;
    }
};

#endif
