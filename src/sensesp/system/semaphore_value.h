#ifndef SENSESP_SRC_SENSESP_SYSTEM_SEMAPHORE_VALUE_H_
#define SENSESP_SRC_SENSESP_SYSTEM_SEMAPHORE_VALUE_H_

#include "sensesp.h"

#include <freeRTOS/semphr.h>

namespace sensesp {

/**
 * @brief A value container wrapped in a semaphore.
 *
 * SemaphoreValue is primarily useful for synchronizing access to a value.
 * It is a regular ValueConsumer that can receive values from a ValueProducer.
 * The value is wrapped in a semaphore which can be waited on.
 * This allows a thread to wait until the value is updated, making
 * SemaphoreValue useful for synchronizing threads.
 *
 * SemaphoreValue is similar to TaskQueueProducer, but it is not a queue and
 * does not poll or emit the values.
 *
 */
template <typename T>
class SemaphoreValue : public ValueConsumer<T> {
 public:
  SemaphoreValue() : ValueConsumer<T>() {
    semaphore_ = xSemaphoreCreateBinary();
  }

  /**
   * @brief Wait for the value to be updated.
   *
   * This method blocks until the value is updated.
   *
   */
  bool wait(T& destination, unsigned int max_duration_ms) {
    if (xSemaphoreTake(semaphore_, max_duration_ms / portTICK_PERIOD_MS) ==
        pdTRUE) {
      destination = value_;
      return true;
    }
    return false;
  }

  /**
   * @brief Take the semaphore, ignoring the value.
   *
   */
  bool take(unsigned int max_duration_ms) {
    return xSemaphoreTake(semaphore_, max_duration_ms / portTICK_PERIOD_MS) ==
           pdTRUE;
  }

  void set(const T& new_value) override {
    value_ = new_value;
    xSemaphoreGive(semaphore_);
  }

  void clear() { xSemaphoreTake(semaphore_, 0); }

 protected:
  T value_;
  SemaphoreHandle_t semaphore_;
};

}  // namespace sensesp

#endif  // SENSESP_SRC_SENSESP_SYSTEM_SEMAPHORE_VALUE_H_
