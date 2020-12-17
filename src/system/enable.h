#ifndef _enable_H_
#define _enable_H_

#include <stdint.h>

#include <queue>

#include "sensesp.h"

/**
 * @brief Classes that implement "Enable" will have their enable() method
 * called automatically at startup when the SensESP app itself
 * is enabled.  The optional priority allows for certain classes
 * to be initialized first. The default priority is zero.
 * A higher priority (i.e. greater than zero) will be enabled before a
 * lower one (i.e. less than zero).
 */
class Enable {
 public:
  Enable(uint8_t priority = 0);

  /**
   * Called during the initialization process.  Override this method
   * to add runtime initialization code to your class
   */
  virtual void enable() {}

  const uint8_t get_enable_priority() { return priority; }

  void set_priority(uint8_t priority) { this->priority = priority; }

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

  // FIXME: Uncomment the following once the PIO Xtensa toolchain is updated
  // [[deprecated("Use enable_all() instead.")]]
  static void enableAll() {
    debugW("Use enable_all() instead.");
    enable_all();
  };

  friend bool operator<(const Enable& lhs, const Enable& rhs) {
    return lhs.priority < rhs.priority;
  }

 private:
  uint8_t priority;

  static std::priority_queue<Enable*> enableList;
};

#endif
