#ifndef SENSESP_SYSTEM_STARTABLE_H
#define SENSESP_SYSTEM_STARTABLE_H

namespace sensesp {

/**
 * @brief Dummy Startable class for backward compatibility.
 *
 */
class Startable {
 public:
  Startable(int priority = 0) {}
  virtual void start() {}
  const int get_start_priority() { return 0; }
  void set_start_priority(int priority) {}
  static void start_all() {}
};

}  // namespace sensesp

#endif  // SENSESP_SRC_SENSESP_SYSTEM_STARTABLE_H_
