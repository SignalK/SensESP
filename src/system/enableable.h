#ifndef _enableable_H_
#define _enableable_H_

#include <queue>

class EnableableCompare;

/**
 * @brief Automatic calling of the enable() method at startup
 * 
 * Classes that implement "Enableable" will have their enable() method
 * called automatically at startup when the SensESP app itself
 * is enabled.  The optional priority allows for certain classes
 * to be initialized first. The default priority is zero.
 * A higher priority (i.e. greater than zero) will be enabled before a
 * lower one (i.e. less than zero).
 */
class Enableable {
 public:
  Enableable(int priority = 0);

  /**
   * Called during the initialization process.  Override this method
   * to add runtime initialization code to your class
   */
  virtual void enable() {}

  const int get_enable_priority() { return priority_; }

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
  static void enable_all();

  friend class EnableableCompare;
 private:
  int priority_;

  static std::priority_queue<Enableable*, std::vector<Enableable*>, EnableableCompare> enable_list_;
};

class EnableableCompare {
  public:
    bool operator()(const Enableable* lhs, const Enableable* rhs) const {
      return lhs->priority_ < rhs->priority_;
    }
};

#endif
