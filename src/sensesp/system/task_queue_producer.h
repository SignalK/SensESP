#ifndef SENSESP_SYSTEM_TASK_QUEUE_PRODUCER_H_
#define SENSESP_SYSTEM_TASK_QUEUE_PRODUCER_H_

#include <limits>
#include <queue>

#include "ReactESP.h"
#include "esp_log.h"
#include "observablevalue.h"
#include "sensesp_base_app.h"

namespace sensesp {

/**
 * @brief Thread-safe queue for inter-task communication.
 *
 * Uses a statically allocated FreeRTOS mutex (no heap allocation) to avoid
 * corruption from heap fragmentation or adjacent stack overflows.
 *
 * Instances must outlive all tasks that access them. Destroying a SafeQueue
 * while another task is blocked on it is undefined behavior.
 *
 * @tparam T
 */
template <typename T>
class SafeQueue {
 public:
  SafeQueue(size_t max_size = 100) : max_size_(max_size) {
    write_lock_ = xSemaphoreCreateMutexStatic(&write_lock_buffer_);
  }

  // Non-copyable, non-movable — the semaphore handle is not transferable.
  SafeQueue(const SafeQueue&) = delete;
  SafeQueue& operator=(const SafeQueue&) = delete;
  SafeQueue(SafeQueue&&) = delete;
  SafeQueue& operator=(SafeQueue&&) = delete;

  void push(const T& value) {
    if (xSemaphoreTake(write_lock_, portMAX_DELAY) == pdTRUE) {
      if (queue_.size() >= max_size_) {
        queue_.pop();
        ESP_LOGW("SafeQueue", "Queue full, dropping oldest entry");
      }
      queue_.push(value);
      xSemaphoreGive(write_lock_);
    }
  }

  // NOTE: max_duration_ms is currently unused but retained for API
  // compatibility. The pop is non-blocking; it returns immediately if
  // the queue is empty.
  bool pop(T& value, unsigned int max_duration_ms) {
    bool result = false;
    if (xSemaphoreTake(write_lock_, portMAX_DELAY) == pdTRUE) {
      if (!queue_.empty()) {
        value = queue_.front();
        queue_.pop();
        result = true;
      }
      xSemaphoreGive(write_lock_);
    }
    return result;
  }

  bool empty() {
    bool result = true;
    if (xSemaphoreTake(write_lock_, portMAX_DELAY) == pdTRUE) {
      result = queue_.empty();
      xSemaphoreGive(write_lock_);
    }
    return result;
  }

  size_t size() {
    size_t result = 0;
    if (xSemaphoreTake(write_lock_, portMAX_DELAY) == pdTRUE) {
      result = queue_.size();
      xSemaphoreGive(write_lock_);
    }
    return result;
  }

 private:
  std::queue<T> queue_;
  size_t max_size_;
  StaticSemaphore_t write_lock_buffer_;
  SemaphoreHandle_t write_lock_;
};

/**
 * @brief Producer class that works across task boundaries.
 *
 * Normal ObservableValues call the observer callbacks within the same
 * task content. In a multi-task software, this is not always preferable.
 * This class allows you to produce values in one task and consume them
 * in another.
 *
 * @tparam T
 * @param consumer_event_loop The event loop in which the values should be
 * consumed.
 * @param poll_rate How often to poll the queue. Note: in microseconds! A value
 * of 0 means that the queue will be polled on every tick.
 */
template <class T>
class TaskQueueProducer : public ObservableValue<T> {
 public:
  TaskQueueProducer(const T& value,
                    std::shared_ptr<reactesp::EventLoop> consumer_event_loop,
                    unsigned int poll_rate = 990)
      : ObservableValue<T>(value) {
    auto func = [this]() {
      T value;
      while (queue_.pop(value, 0)) {
        this->emit(value);
      }
    };

    // Create a repeat event that will poll the queue and emit the values
    if (poll_rate == 0) {
      consumer_event_loop->onTick(func);
    } else {
      consumer_event_loop->onRepeatMicros(poll_rate, func);
    }
  }

  TaskQueueProducer(const T& value, unsigned int poll_rate = 990)
      : TaskQueueProducer(value, event_loop(), poll_rate) {}

  virtual void set(const T& value) override { queue_.push(value); }

  /**
   * @brief Wait for a value to be available in the queue.
   *
   * This function will block until a value is available in the queue. When a
   * value becomes available, it will be returned in the reference and
   * emitted to the observers.
   *
   * WARNING: wait() calls emit(), which invokes all observers. This is
   * only safe if wait() is called from the same FreeRTOS task where
   * observers were registered (typically the main event loop task).
   * For cross-task use, rely on the polling lambda set up in the constructor.
   *
   * @param value Received value if the function returns true.
   * @param max_duration_ms Maximum duration to wait for the value.
   * @return true Value was received successfully.
   * @return false
   */
  bool wait(T& value, unsigned int max_duration_ms) {
    T received_value;
    bool result = queue_.pop(received_value, max_duration_ms);
    if (result) {
      value = received_value;
      this->emit(value);
    }
    return result;
  }

 private:
  SafeQueue<T> queue_;
};

}  // namespace sensesp

#endif  // SENSESP_SYSTEM_TASK_QUEUE_PRODUCER_H_
