#ifndef SENSESP_SYSTEM_TASK_QUEUE_PRODUCER_H_
#define SENSESP_SYSTEM_TASK_QUEUE_PRODUCER_H_

#include "ReactESP.h"
#include "observablevalue.h"

namespace sensesp {

/**
 * @brief Producer class that works across task boundaries.
 *
 * Normal ObservableValues call the observer callbacks within the same
 * task content. In a multi-task software, this is not always preferable.
 * This class allows you to produce values in one task and consume them
 * in another.
 *
 * @tparam T
 * @param consumer_app The app object in which the values should be consumed.
 * @param queue_size Size of the queue.
 * @param poll_rate How often to poll the queue. Note: in microseconds!
 */
template <class T>
class TaskQueueProducer : public ObservableValue<T> {
 public:
  TaskQueueProducer(const T& value, reactesp::ReactESP* consumer_app = ReactESP::app, int queue_size = 1,
                    unsigned int poll_rate = 990)
      : ObservableValue<T>(value), queue_size_{queue_size} {
    queue_ = xQueueCreate(queue_size, sizeof(T));
    if (queue_ == NULL) {
      debugE("Failed to create queue");
    }

    // Create a repeat reaction that will poll the queue and emit the values
    consumer_app->onRepeatMicros(poll_rate, [this]() {
      T value;
      while (xQueueReceive(queue_, &value, 0) == pdTRUE) {
        this->emit(value);
      }
    });
  }

  TaskQueueProducer(const T& value, int queue_size = 1, unsigned int poll_rate = 990)
      : TaskQueueProducer(value, ReactESP::app, queue_size, poll_rate) {}

  virtual void set(const T& value) override {
    int retval;
    if (queue_size_ == 1) {
      retval = xQueueOverwrite(queue_, &value);
    } else {
      retval = xQueueSend(queue_, &value, 0);
    }
  }

 private:
  int queue_size_;
  QueueHandle_t queue_;
};

}  // namespace sensesp

#endif  // SENSESP_SYSTEM_TASK_QUEUE_PRODUCER_H_
