#ifndef SENSESP_SRC_SENSESP_SIGNALK_SIGNALK_WS_MESSAGE_ASSEMBLER_H_
#define SENSESP_SRC_SENSESP_SIGNALK_SIGNALK_WS_MESSAGE_ASSEMBLER_H_

#include <cstddef>
#include <cstdint>
#include <string>

namespace sensesp {

/**
 * @brief Puts a received WebSocket message back together from the pieces
 * esp_websocket_client posts it in.
 *
 * esp_websocket_client posts one WEBSOCKET_EVENT_DATA per transport read, of
 * at most its buffer_size (SENSESP_SK_WS_BUFFER_SIZE, 1024 by default). A
 * frame larger than that, or one TCP delivered in parts, arrives as several
 * events, each giving its place in the frame (payload_offset of payload_len).
 * A message the server fragmented arrives as several frames, the later ones
 * continuation frames (opcode 0) and the last with fin set. Parsing each
 * event on its own loses the message: the first piece fails with
 * IncompleteInput, the next with InvalidInput.
 *
 * Pure and dependency-free (no Arduino, no esp_websocket_client) so it can be
 * unit-tested on the host. Not thread-safe: feed it from the client's event
 * handler only, which esp_websocket_client calls from its one task.
 */
class SKWSMessageAssembler {
 public:
  enum class Result {
    kPartial,   ///< Part of a message: more to come
    kComplete,  ///< message() holds a whole message
    kDropped,   ///< A message was lost: too large, or a piece out of place
    kIgnored,   ///< A control frame (ping, pong, close): no message data
  };

  /// @param max_bytes The largest message kept; a larger one is dropped.
  explicit SKWSMessageAssembler(size_t max_bytes) : max_bytes_{max_bytes} {}

  /// Forget any message in progress (call on every new connection).
  void reset() {
    start_over();
    // Hand a buffer grown by a rare large message back to the heap. Only
    // here, not between messages, so a run of large messages reuses it.
    if (message_.capacity() > kKeptCapacity) {
      message_.shrink_to_fit();
    }
  }

  /**
   * One WEBSOCKET_EVENT_DATA: the frame's opcode and fin flag, the frame's
   * payload length, this piece's offset in it, and the piece itself. After
   * kComplete, message() holds the message until the next call.
   */
  Result add(uint8_t op_code, bool fin, int payload_len, int payload_offset,
             const char* data, int data_len) {
    if (op_code >= 0x8) {
      return Result::kIgnored;  // Control frames may come between fragments
    }
    if (data_len < 0 || payload_offset < 0 || payload_len < 0 ||
        (data_len > 0 && data == nullptr)) {
      return lose();
    }
    if (payload_offset == 0) {
      // The first piece of a frame. A text or binary frame starts a message
      // (giving up an unfinished one before it); a continuation frame
      // carries the current one on.
      if (op_code != 0x0) {
        start_over();
        in_message_ = true;
      } else if (!in_message_) {
        return Result::kDropped;  // A continuation of nothing we hold
      }
      frame_pos_ = 0;
    } else if (!in_message_ ||
               static_cast<size_t>(payload_offset) != frame_pos_) {
      return lose();  // A piece out of place: what we hold is broken
    }
    frame_pos_ += static_cast<size_t>(data_len);
    if (!dropping_) {
      if (message_.size() + static_cast<size_t>(data_len) > max_bytes_) {
        message_.clear();
        dropping_ = true;  // Skip the rest of it, report it once at its end
      } else {
        message_.append(data, static_cast<size_t>(data_len));
      }
    }
    const bool frame_done =
        frame_pos_ >= static_cast<size_t>(payload_len);
    if (!frame_done || !fin) {
      return Result::kPartial;
    }
    in_message_ = false;
    if (dropping_) {
      dropping_ = false;
      return Result::kDropped;
    }
    return Result::kComplete;
  }

  /// The message after add() returned kComplete.
  const std::string& message() const { return message_; }

 private:
  static constexpr size_t kKeptCapacity = 2048;

  /// Forget the message in progress, keeping the buffer for the next one.
  void start_over() {
    message_.clear();
    in_message_ = false;
    dropping_ = false;
    frame_pos_ = 0;
  }

  Result lose() {
    start_over();
    return Result::kDropped;
  }

  size_t max_bytes_;
  std::string message_;
  bool in_message_ = false;
  bool dropping_ = false;  ///< Too large: the rest of it is skipped
  size_t frame_pos_ = 0;   ///< Bytes of the current frame received so far
};

}  // namespace sensesp

#endif  // SENSESP_SRC_SENSESP_SIGNALK_SIGNALK_WS_MESSAGE_ASSEMBLER_H_
