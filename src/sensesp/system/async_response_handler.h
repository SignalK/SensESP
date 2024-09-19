#ifndef SENSESP_SRC_SENSESP_SIGNALK_SYSTEM_ASYNC_RESPONSE_HANDLER_H_
#define SENSESP_SRC_SENSESP_SIGNALK_SYSTEM_ASYNC_RESPONSE_HANDLER_H_

#include "sensesp.h"

#include <Arduino.h>
#include <elapsedMillis.h>

#include "sensesp_base_app.h"
#include "valueproducer.h"

namespace sensesp {

enum class AsyncResponseStatus {
  kReady,
  kPending,
  kSuccess,
  kFailure,
  kTimeout,
};

/**
 * @brief Handle async command responses from remote systems.
 *
 * This class is used to report responses to commands sent to a remote system.
 * The class should consume a boolean value when a response is received. The
 * boolean value indicates whether the command was successful or not.
 * The response handler also emits the response status to its subscribers.
 *
 */
class AsyncResponseHandler : public ValueConsumer<bool>,
                             public ValueProducer<AsyncResponseStatus> {
 public:
  AsyncResponseHandler()
      : ValueConsumer<bool>(), ValueProducer<AsyncResponseStatus>() {}
  AsyncResponseHandler(int timeout)
      : ValueConsumer<bool>(),
        ValueProducer<AsyncResponseStatus>(),
        timeout_{timeout} {}

  /**
   * @brief Activate must be called when a command is sent to the remote system.
   *
   * @param value
   */
  void activate() {
    ESP_LOGV("AsyncResponseHandler", "Activating response handler");
    elapsed_millis_ = 0;
    status_ = AsyncResponseStatus::kPending;
    this->emit(status_);

    if (timeout_event_ != nullptr) {
      SensESPBaseApp::get_event_loop()->remove(timeout_event_);
      timeout_event_ = nullptr;
    }
    timeout_event_ =
        SensESPBaseApp::get_event_loop()->onDelay(timeout_, [this]() {
          if (status_ == AsyncResponseStatus::kPending) {
            ESP_LOGV("AsyncResponseHandler", "Timeout");
            status_ = AsyncResponseStatus::kTimeout;
            this->emit(status_);
          }
          this->timeout_event_ = nullptr;
        });
  }

  void set(const bool& success) override {
    ESP_LOGV("AsyncResponseHandler", "status: %d", status_);
    if (status_ != AsyncResponseStatus::kPending) {
      ESP_LOGV("AsyncResponseHandler",
               "Received response when not in pending state");
      return;
    }

    // Clear the timeout event
    if (timeout_event_ != nullptr) {
      SensESPBaseApp::get_event_loop()->remove(timeout_event_);
      timeout_event_ = nullptr;
    }

    ESP_LOGV("AsyncResponseHandler", "Received response: %d", success);

    if (success) {
      status_ = AsyncResponseStatus::kSuccess;
    } else {
      status_ = AsyncResponseStatus::kFailure;
    }
    this->emit(status_);
  }

  const AsyncResponseStatus get_status() const { return status_; }

 protected:
  reactesp::DelayEvent* timeout_event_ = nullptr;
  AsyncResponseStatus status_ = AsyncResponseStatus::kReady;
  String result_message_;
  int timeout_ = 3000;  // Default timeout in ms
  elapsedMillis elapsed_millis_;
};

}  // namespace sensesp

#endif  // SENSESP_SRC_SENSESP_SIGNALK_SYSTEM_ASYNC_RESPONSE_HANDLER_H_
