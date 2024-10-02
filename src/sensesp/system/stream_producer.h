#ifndef SENSESP_SYSTEM_STREAM_PRODUCER_H
#define SENSESP_SYSTEM_STREAM_PRODUCER_H

#include "sensesp.h"

#include "ReactESP.h"
#include "sensesp/system/valueproducer.h"
#include "sensesp_base_app.h"

namespace sensesp {

/**
 * @brief ValueProducer that reads from a Stream and produces each character.
 */
class StreamCharProducer : public ValueProducer<char> {
 public:
  StreamCharProducer(Stream* stream) : stream_{stream} {
    read_event_ =
        event_loop()->onAvailable(*stream_, [this]() {
          while (stream_->available()) {
            char c = stream_->read();
            this->emit(c);
          }
        });
  }

 protected:
  Stream* stream_;
  reactesp::StreamEvent* read_event_;
};

/**
 * @brief ValueProducer that reads from a Stream and produces a full line.
 */
class StreamLineProducer : public ValueProducer<String> {
 public:
  StreamLineProducer(Stream* stream, int max_line_length = 256)
      : stream_{stream}, max_line_length_{max_line_length} {
    static int buf_pos = 0;
    buf_ = new char[max_line_length_ + 1];
    read_event_ =
        event_loop()->onAvailable(*stream_, [this]() {
          while (stream_->available()) {
            char c = stream_->read();
            if (c == '\n') {
              // Include the newline character in the output
              buf_[buf_pos++] = c;
              buf_[buf_pos] = '\0';
              this->emit(buf_);
              buf_pos = 0;
            } else {
              buf_[buf_pos++] = c;
              if (buf_pos >= max_line_length_ - 1) {
                buf_pos = 0;
              }
            }
          }
        });
  }

 protected:
  const int max_line_length_;
  char* buf_;
  Stream* stream_;
  reactesp::StreamEvent* read_event_;
};

}  // namespace sensesp

#endif  // SENSESP_SRC_SENSESP_SYSTEM_STREAM_PRODUCER_H_
