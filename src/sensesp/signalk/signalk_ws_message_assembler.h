#ifndef SENSESP_SRC_SENSESP_SIGNALK_SIGNALK_WS_MESSAGE_ASSEMBLER_H_
#define SENSESP_SRC_SENSESP_SIGNALK_SIGNALK_WS_MESSAGE_ASSEMBLER_H_

#include <cstddef>
#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>

namespace sensesp {

/// Whether a received frame is the last of its message.
enum class WSFrameEnd {
  kMore,     ///< fin clear: continuation frames follow
  kLast,     ///< fin set: the message ends with this frame
  kUnknown,  ///< The client does not say (ESP-IDF 4.4's bundled client)
};

namespace ws_detail {
// Detects a `fin` member, so the build adapts to the client it is given
// rather than guessing from a version number. C++11: the Arduino platform
// builds with gnu++11, so no std::void_t or if constexpr.
/// False unless T has a `fin` member (the specialisation below).
template <typename T, typename = void>
struct HasFin : std::false_type {};
/// Chosen only when `event.fin` is a valid expression for T.
template <typename T>
struct HasFin<T, decltype(void(std::declval<const T&>().fin))>
    : std::true_type {};

/// The client reports fin: say whether more frames follow.
template <typename Event>
WSFrameEnd frame_end(const Event& event, std::true_type) {
  return event.fin ? WSFrameEnd::kLast : WSFrameEnd::kMore;
}
/// The client has no fin flag; never instantiates `event.fin`.
template <typename Event>
WSFrameEnd frame_end(const Event&, std::false_type) {
  return WSFrameEnd::kUnknown;
}
}  // namespace ws_detail

/**
 * @brief The fin flag of an esp_websocket_event_data_t, if the client reports
 * one. The client bundled with ESP-IDF 4.4 discards it while parsing the frame
 * header; the esp_websocket_client component (ESP-IDF 5) reports it. Any other
 * client is handled by what its struct holds.
 */
template <typename Event>
WSFrameEnd ws_frame_end(const Event& event) {
  return ws_detail::frame_end(event, ws_detail::HasFin<Event>{});
}

/**
 * @brief Tells, byte by byte, when a JSON text has ended: its outermost
 * object or array has closed. Keeps only a little state between pieces, so it
 * reads each byte once however the message is split.
 *
 * Stands in for the fin flag where the client does not report one: Signal K
 * sends only JSON objects, so the message ends where its top-level object
 * does. A text that is not an object or array (a bare number, say) is taken
 * to end with its frame.
 */
class JsonEndScanner {
 public:
  /// Start on a new message.
  void reset() { *this = JsonEndScanner{}; }

  /// Read the next `len` bytes of the message; stops reading once it closes.
  void feed(const char* data, size_t len) {
    for (size_t i = 0; i < len && !closed_; i++) {
      const char c = data[i];
      if (in_string_) {
        if (escaped_) {
          escaped_ = false;
        } else if (c == '\\') {
          escaped_ = true;
        } else if (c == '"') {
          in_string_ = false;
          closed_ = depth_ <= 0;  // A top-level string has ended
        }
        continue;
      }
      if (!started_) {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') continue;
        started_ = true;
        if (c != '{' && c != '[') {
          scalar_ = true;
          in_string_ = c == '"';
          continue;
        }
      }
      switch (c) {
        case '"':
          in_string_ = true;
          break;
        case '{':
        case '[':
          depth_++;
          break;
        case '}':
        case ']':
          // Below zero is malformed: end it here and let the parser say so.
          closed_ = --depth_ <= 0;
          break;
        default:
          break;
      }
    }
  }

  /// Whether the text read so far is a whole JSON value.
  bool complete() const { return closed_ || (scalar_ && !in_string_); }

 private:
  int depth_ = 0;
  bool started_ = false;
  bool scalar_ = false;  ///< The top-level value is not an object or array
  bool in_string_ = false;
  bool escaped_ = false;
  bool closed_ = false;
};

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
 * Where the client does not report fin (WSFrameEnd::kUnknown), a message ends
 * at the end of the frame in which its JSON closes (JsonEndScanner); a frame
 * that leaves it open waits for continuation frames. Either way, the size
 * limit applies to the whole message, and nothing is handed on before the
 * message is whole.
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
   * One WEBSOCKET_EVENT_DATA: the frame's opcode and whether it ends the
   * message (see ws_frame_end()), the frame's payload length, this piece's
   * offset in it, and the piece itself. After kComplete, message() holds the
   * message until the next call.
   */
  Result add(uint8_t op_code, WSFrameEnd frame_end, int payload_len,
             int payload_offset, const char* data, int data_len) {
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
    // Read even while dropping, to find the end of the message being skipped.
    json_end_.feed(data, static_cast<size_t>(data_len));
    if (!dropping_) {
      if (message_.size() + static_cast<size_t>(data_len) > max_bytes_) {
        message_.clear();
        dropping_ = true;  // Skip the rest of it, report it once at its end
      } else {
        message_.append(data, static_cast<size_t>(data_len));
      }
    }
    const bool frame_done = frame_pos_ >= static_cast<size_t>(payload_len);
    const bool message_done =
        frame_end == WSFrameEnd::kLast ||
        (frame_end == WSFrameEnd::kUnknown && json_end_.complete());
    if (!frame_done || !message_done) {
      return Result::kPartial;
    }
    in_message_ = false;
    if (dropping_) {
      dropping_ = false;
      return Result::kDropped;
    }
    return Result::kComplete;
  }

  /// add() for a client that reports fin.
  Result add(uint8_t op_code, bool fin, int payload_len, int payload_offset,
             const char* data, int data_len) {
    return add(op_code, fin ? WSFrameEnd::kLast : WSFrameEnd::kMore,
               payload_len, payload_offset, data, data_len);
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
    json_end_.reset();
  }

  /// Give up what is held (it can no longer be whole) and report it lost.
  Result lose() {
    start_over();
    return Result::kDropped;
  }

  size_t max_bytes_;
  std::string message_;
  bool in_message_ = false;
  bool dropping_ = false;    ///< Too large: the rest of it is skipped
  size_t frame_pos_ = 0;     ///< Bytes of the current frame received so far
  JsonEndScanner json_end_;  ///< Where the message ends, when fin is unknown
};

}  // namespace sensesp

#endif  // SENSESP_SRC_SENSESP_SIGNALK_SIGNALK_WS_MESSAGE_ASSEMBLER_H_
