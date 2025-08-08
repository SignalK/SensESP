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
 * @brief Thread-safe queue for inter-task communication. Works like std::queue.
 *
 * @tparam T
 */
template <typename T>
class SafeQueue : public std::queue<T> {
 public:
  SafeQueue() : std::queue<T>() {
    write_lock_ = xSemaphoreCreateMutex();
    if (write_lock_ == nullptr) {
      // Handle semaphore creation failure
      ESP_LOGE("SafeQueue", "Failed to create mutex semaphore");
    }
  }

  ~SafeQueue() {
    if (write_lock_ != nullptr) {
      vSemaphoreDelete(write_lock_);
    }
  }

  void push(const T& value) {
    if (write_lock_ == nullptr) return;
    if (xSemaphoreTake(write_lock_, portMAX_DELAY) == pdTRUE) {
      std::queue<T>::push(value);
      xSemaphoreGive(write_lock_);
    }
  }

  bool pop(T& value, unsigned int max_duration_ms) {
    if (write_lock_ == nullptr) {
      return false;
    }
    bool result = false;
    if (xSemaphoreTake(write_lock_, portMAX_DELAY) == pdTRUE) {
      if (!std::queue<T>::empty()) {
        value = std::queue<T>::front();
        std::queue<T>::pop();
        result = true;
      }
      xSemaphoreGive(write_lock_);
    }
    return result;
  }

  bool empty() {
    if (write_lock_ == nullptr) {
      return true;
    }
    bool result = true;
    if (xSemaphoreTake(write_lock_, portMAX_DELAY) == pdTRUE) {
      result = std::queue<T>::empty();
      xSemaphoreGive(write_lock_);
    }
    return result;
  }

  size_t size() {
    if (write_lock_ == nullptr) {
      return 0;
    }
    size_t result = 0;
    if (xSemaphoreTake(write_lock_, portMAX_DELAY) == pdTRUE) {
      result = std::queue<T>::size();
      xSemaphoreGive(write_lock_);
    }
    return result;
  }

 protected:
  SemaphoreHandle_t write_lock_;  // Lock for writing to the queue
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
