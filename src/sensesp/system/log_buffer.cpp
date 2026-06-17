#include "sensesp/system/log_buffer.h"

#include <esp_log.h>
#include <esp_random.h>
#include <esp_timer.h>

#include <cstdio>
#include <cstring>

#ifndef USE_ESP_IDF_LOG
#warning \
    "USE_ESP_IDF_LOG is not defined: Arduino-ESP32 reroutes ESP_LOGx to " \
    "log_printf, which bypasses esp_log_set_vprintf. The web log viewer will " \
    "capture nothing. Build with -D USE_ESP_IDF_LOG."
#endif

namespace sensesp {

LogBuffer* LogBuffer::instance_ = nullptr;

LogBuffer::LogBuffer(size_t max_lines, uint32_t max_age_ms,
                     size_t max_line_length)
    : max_lines_(max_lines),
      max_age_ms_(max_age_ms),
      max_line_length_(max_line_length),
      session_id_(0) {
  mutex_ = xSemaphoreCreateMutexStatic(&mutex_buffer_);
}

void LogBuffer::install() {
  // Draw the per-boot session id here rather than in the constructor: the hook
  // is installed during setup(), past early static init, and mixing in the boot
  // timer makes a repeated value across reboots unlikely even before the RF
  // subsystem seeds the RNG. The client also falls back to a cursor reset.
  session_id_ = esp_random() ^ static_cast<uint32_t>(esp_timer_get_time());

  // Stand up the handoff queue and capture task before the hook goes live, so
  // the first captured line already has somewhere to go.
  capture_queue_ = xQueueCreate(kLogCaptureQueueDepth, sizeof(LogQueueItem));
  if (capture_queue_ != nullptr) {
    xTaskCreate(&LogBuffer::capture_task, "LogCapture", 4096, this, 1,
                &capture_task_handle_);
  }

  instance_ = this;
  previous_vprintf_ = esp_log_set_vprintf(&LogBuffer::vprintf_trampoline);
}

void LogBuffer::capture_task(void* arg) {
  auto* self = static_cast<LogBuffer*>(arg);
  LogQueueItem item;
  while (true) {
    if (xQueueReceive(self->capture_queue_, &item, portMAX_DELAY) == pdTRUE) {
      // The heap-allocating work (ANSI strip, std::string, deque) happens here,
      // on this task's stack, not the logging task's.
      self->push_line(item.text, item.length, item.timestamp_ms);
    }
  }
}

int LogBuffer::vprintf_trampoline(const char* format, va_list args) {
  LogBuffer* inst = instance_;

  // Forward to the previously installed handler so UART output is unchanged.
  // A va_list can be traversed only once, so each consumer gets its own copy.
  int ret = 0;
  if (inst != nullptr && inst->previous_vprintf_ != nullptr) {
    va_list copy;
    va_copy(copy, args);
    ret = inst->previous_vprintf_(format, copy);
    va_end(copy);
  }

  // Capture path. This runs in the logging task's context, so it must stay
  // light on stack and never block or allocate: format into a stack item and
  // hand it to the capture task, which does the heap work on its own stack.
  // Skipped in ISR context (we would not block there anyway); the line was
  // already forwarded to UART above.
  if (inst != nullptr && inst->capture_queue_ != nullptr &&
      !xPortInIsrContext()) {
    LogQueueItem item;
    va_list copy;
    va_copy(copy, args);
    int n = vsnprintf(item.text, sizeof(item.text), format, copy);
    va_end(copy);
    if (n > 0) {
      item.length = (static_cast<size_t>(n) < sizeof(item.text))
                        ? static_cast<uint16_t>(n)
                        : sizeof(item.text) - 1;
      item.timestamp_ms = esp_log_timestamp();
      // Non-blocking: drop the line if the capture task is behind rather than
      // stall the logging task.
      xQueueSend(inst->capture_queue_, &item, 0);
    }
  }

  return ret;
}

namespace {
// Copy text with ANSI/VT100 escape sequences removed. ESP_LOG wraps each line
// in color codes when CONFIG_LOG_COLORS is enabled; the raw ESC byte (0x1b) is
// a control character that ArduinoJson does not escape, so it would otherwise
// reach the /api/log response unescaped and make the JSON unparseable in the
// browser. A CSI sequence is ESC '[' then parameter/intermediate bytes up to a
// final byte in 0x40-0x7e; a lone ESC is simply dropped.
std::string strip_ansi(const char* text, size_t length) {
  std::string out;
  out.reserve(length);
  size_t i = 0;
  while (i < length) {
    if (text[i] == '\x1b') {
      i++;  // drop ESC
      if (i < length && text[i] == '[') {
        i++;  // drop '['
        while (i < length) {
          unsigned char c = static_cast<unsigned char>(text[i]);
          i++;
          if (c >= 0x40 && c <= 0x7e) {
            break;  // final byte, end of sequence
          }
        }
      }
      continue;
    }
    out.push_back(text[i]);
    i++;
  }
  return out;
}
}  // namespace

void LogBuffer::push_line(const char* text, size_t length, uint32_t now_ms) {
  // Strip trailing CR/LF so each record is one display line.
  while (length > 0 && (text[length - 1] == '\n' || text[length - 1] == '\r')) {
    length--;
  }
  if (length == 0) {
    return;
  }

  // Remove ANSI color escapes so the buffered copy served over /api/log is
  // plain, JSON-safe text. The console output above keeps its colors.
  std::string line = strip_ansi(text, length);
  if (line.empty()) {
    return;
  }
  if (line.size() > max_line_length_) {
    line.resize(max_line_length_);
  }

  xSemaphoreTake(mutex_, portMAX_DELAY);
  prune_locked(now_ms);
  records_.push_back({next_seq_, now_ms, std::move(line)});
  next_seq_++;
  // Enforce the count cap after inserting the new record.
  while (records_.size() > max_lines_) {
    records_.pop_front();
  }
  xSemaphoreGive(mutex_);
}

void LogBuffer::prune_locked(uint32_t now_ms) {
  while (!records_.empty()) {
    const LogRecord& front = records_.front();
    if (now_ms >= front.timestamp_ms &&
        now_ms - front.timestamp_ms > max_age_ms_) {
      records_.pop_front();
    } else {
      break;
    }
  }
}

LogSnapshot LogBuffer::snapshot_since(uint32_t since, bool has_since,
                                      uint32_t now_ms) {
  LogSnapshot snapshot;
  snapshot.session_id = session_id_;
  snapshot.gap = false;

  xSemaphoreTake(mutex_, portMAX_DELAY);
  prune_locked(now_ms);
  snapshot.next = next_seq_;

  if (!has_since) {
    // Initial load: return the whole retained buffer, never a gap.
    for (const auto& record : records_) {
      snapshot.lines.push_back(record.text);
    }
  } else {
    if (records_.empty()) {
      // Lines existed between the client's cursor and now but were all
      // evicted. (since >= next_seq_ is the caught-up or post-reboot case.)
      snapshot.gap = since < next_seq_;
    } else if (records_.front().seq > since) {
      // The line the client expected next was evicted before it was read.
      snapshot.gap = true;
    }
    for (const auto& record : records_) {
      if (record.seq >= since) {
        snapshot.lines.push_back(record.text);
      }
    }
  }
  xSemaphoreGive(mutex_);

  return snapshot;
}

LogSnapshot LogBuffer::snapshot_since(uint32_t since, bool has_since) {
  return snapshot_since(since, has_since, esp_log_timestamp());
}

size_t LogBuffer::size() {
  xSemaphoreTake(mutex_, portMAX_DELAY);
  size_t count = records_.size();
  xSemaphoreGive(mutex_);
  return count;
}

}  // namespace sensesp
