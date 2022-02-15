#include "signalk_listener.h"

namespace sensesp {

std::vector<SKListener*> SKListener::listeners;

SemaphoreHandle_t SKListener::semaphore_ = xSemaphoreCreateRecursiveMutex();

SKListener::SKListener(String sk_path, int listen_delay)
    : sk_path{sk_path}, listen_delay{listen_delay} {
  listeners.push_back(this);
}

bool SKListener::take_semaphore(unsigned long int timeout_ms) {
  if (timeout_ms == 0) {
    return xSemaphoreTakeRecursive(semaphore_, portMAX_DELAY) == pdTRUE;
  } else {
    return xSemaphoreTakeRecursive(semaphore_, timeout_ms) == pdTRUE;
  }
}

void SKListener::release_semaphore() { xSemaphoreGiveRecursive(semaphore_); }


}  // namespace sensesp
