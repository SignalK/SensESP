/**
 * @file ws_message_assembler_test.cpp
 * @brief Host tests for SKWSMessageAssembler: a received WebSocket message
 * put back together from the pieces esp_websocket_client posts it in.
 *
 * Without it, a message larger than SENSESP_SK_WS_BUFFER_SIZE (or one TCP
 * split) was parsed piece by piece and lost: "deserializeJson error:
 * IncompleteInput" followed by "InvalidInput".
 *
 *   pio test -e native -f native/test_ws_message_assembler
 */

#include <algorithm>
#include <string>
#include <unity.h>

#include "sensesp/signalk/signalk_ws_message_assembler.h"

using namespace sensesp;
using Result = SKWSMessageAssembler::Result;

namespace {

constexpr uint8_t kText = 0x1;
constexpr uint8_t kCont = 0x0;
constexpr uint8_t kPing = 0x9;

constexpr WSFrameEnd kUnknown = WSFrameEnd::kUnknown;

/// Feeds `frame` as one frame in pieces of at most `piece` bytes, as the
/// client does with a receive buffer of that size. Returns the last result.
Result feed(SKWSMessageAssembler& a, uint8_t op, WSFrameEnd end,
            const std::string& frame, size_t piece = 1024) {
  Result r = Result::kPartial;
  size_t offset = 0;
  do {
    const size_t n = std::min(piece, frame.size() - offset);
    r = a.add(op, end, static_cast<int>(frame.size()), static_cast<int>(offset),
              frame.data() + offset, static_cast<int>(n));
    offset += n;
  } while (offset < frame.size());
  return r;
}

/// feed() for a frame whose fin flag is known.
Result feed(SKWSMessageAssembler& a, uint8_t op, bool fin,
            const std::string& frame, size_t piece = 1024) {
  return feed(a, op, fin ? WSFrameEnd::kLast : WSFrameEnd::kMore, frame, piece);
}

/// Stand-ins for esp_websocket_event_data_t with and without a fin member.
struct EventWithFin {
  bool fin;
  uint8_t op_code;
};
struct EventWithoutFin {  // ESP-IDF 4.4's bundled client
  uint8_t op_code;
};

/// Whether JsonEndScanner calls `text` whole when fed `piece` bytes at a
/// time.
bool scans_complete(const std::string& text, size_t piece = 1) {
  JsonEndScanner s;
  for (size_t i = 0; i < text.size(); i += piece) {
    s.feed(text.data() + i, std::min(piece, text.size() - i));
  }
  return s.complete();
}

/// A delta-shaped JSON text of about `size` bytes.
std::string delta(size_t size) {
  std::string s = "{\"updates\":[";
  while (s.size() + 3 < size) s += "1,";
  s += "1]}";
  return s;
}

}  // namespace

/// The common case before the fix, too: it must not regress.
void test_whole_message_in_one_piece(void) {
  SKWSMessageAssembler a(4096);
  const std::string m = "{\"updates\":[]}";
  TEST_ASSERT_TRUE(feed(a, kText, true, m) == Result::kComplete);
  TEST_ASSERT_EQUAL_STRING(m.c_str(), a.message().c_str());
}

/// The bug: a frame over the 1 KB receive buffer.
void test_frame_larger_than_the_buffer_is_joined(void) {
  SKWSMessageAssembler a(4096);
  const std::string m = delta(2500);
  size_t offset = 0;
  int pieces = 0;
  Result r = Result::kPartial;
  while (offset < m.size()) {
    const size_t n = std::min<size_t>(1024, m.size() - offset);
    r = a.add(kText, true, static_cast<int>(m.size()), static_cast<int>(offset),
              m.data() + offset, static_cast<int>(n));
    offset += n;
    pieces++;
    if (offset < m.size()) TEST_ASSERT_TRUE(r == Result::kPartial);
  }
  TEST_ASSERT_EQUAL(3, pieces);
  TEST_ASSERT_TRUE(r == Result::kComplete);
  TEST_ASSERT_EQUAL_STRING(m.c_str(), a.message().c_str());
}

/// A small message TCP delivered in two reads.
void test_uneven_pieces_from_tcp_are_joined(void) {
  SKWSMessageAssembler a(4096);
  const std::string m = delta(700);
  const int len = static_cast<int>(m.size());
  TEST_ASSERT_TRUE(a.add(kText, true, len, 0, m.data(), 300) ==
                   Result::kPartial);
  TEST_ASSERT_TRUE(a.add(kText, true, len, 300, m.data() + 300, len - 300) ==
                   Result::kComplete);
  TEST_ASSERT_EQUAL_STRING(m.c_str(), a.message().c_str());
}

/// Fragments, with a control frame between them (RFC 6455 allows it).
void test_fragmented_message_with_a_ping_between(void) {
  SKWSMessageAssembler a(4096);
  const std::string m = delta(3000);
  TEST_ASSERT_TRUE(feed(a, kText, false, m.substr(0, 1200)) ==
                   Result::kPartial);
  TEST_ASSERT_TRUE(a.add(kPing, true, 0, 0, nullptr, 0) == Result::kIgnored);
  TEST_ASSERT_TRUE(feed(a, kCont, false, m.substr(1200, 1000)) ==
                   Result::kPartial);
  TEST_ASSERT_TRUE(feed(a, kCont, true, m.substr(2200)) == Result::kComplete);
  TEST_ASSERT_EQUAL_STRING(m.c_str(), a.message().c_str());
}

/// The buffer of one message never leaks into the next.
void test_messages_one_after_another(void) {
  SKWSMessageAssembler a(4096);
  const std::string m1 = delta(1500), m2 = delta(40);
  TEST_ASSERT_TRUE(feed(a, kText, true, m1) == Result::kComplete);
  TEST_ASSERT_EQUAL_STRING(m1.c_str(), a.message().c_str());
  TEST_ASSERT_TRUE(feed(a, kText, true, m2) == Result::kComplete);
  TEST_ASSERT_EQUAL_STRING(m2.c_str(), a.message().c_str());
}

/// Over the limit: dropped once, at its end, and the next message is fine.
void test_too_large_a_message_is_dropped_once(void) {
  SKWSMessageAssembler a(4096);
  const std::string big = delta(10000), next = delta(100);
  size_t offset = 0;
  int dropped = 0;
  while (offset < big.size()) {
    const size_t n = std::min<size_t>(1024, big.size() - offset);
    const Result r = a.add(kText, true, static_cast<int>(big.size()),
                           static_cast<int>(offset), big.data() + offset,
                           static_cast<int>(n));
    if (r == Result::kDropped) dropped++;
    TEST_ASSERT_TRUE(r != Result::kComplete);
    offset += n;
  }
  TEST_ASSERT_EQUAL(1, dropped);
  TEST_ASSERT_TRUE(feed(a, kText, true, next) == Result::kComplete);
  TEST_ASSERT_EQUAL_STRING(next.c_str(), a.message().c_str());
}

/// The limit is inclusive, matching on_receive_delta()'s own check.
void test_message_of_exactly_the_limit_is_kept(void) {
  SKWSMessageAssembler a(4096);
  const std::string m(4096, 'x');
  TEST_ASSERT_TRUE(feed(a, kText, true, m) == Result::kComplete);
  TEST_ASSERT_EQUAL(4096, a.message().size());
}

/// A piece missing loses that message, not the next one.
void test_a_piece_out_of_place_loses_only_that_message(void) {
  SKWSMessageAssembler a(4096);
  const std::string m = delta(2500), next = delta(100);
  const int len = static_cast<int>(m.size());
  TEST_ASSERT_TRUE(a.add(kText, true, len, 0, m.data(), 1024) ==
                   Result::kPartial);
  TEST_ASSERT_TRUE(a.add(kText, true, len, 2048, m.data() + 2048, len - 2048) ==
                   Result::kDropped);
  TEST_ASSERT_TRUE(feed(a, kText, true, next) == Result::kComplete);
  TEST_ASSERT_EQUAL_STRING(next.c_str(), a.message().c_str());
}

/// The tail of a message lost to a reconnect, say: reported, then the next
/// message is unaffected.
void test_continuation_without_a_start_is_dropped(void) {
  SKWSMessageAssembler a(4096);
  TEST_ASSERT_TRUE(feed(a, kCont, true, "1]}") == Result::kDropped);
  TEST_ASSERT_TRUE(feed(a, kText, true, "{}") == Result::kComplete);
}

/// RFC 6455 forbids a new message before the last fragment, but a server
/// that sends one must not wedge the assembler.
void test_an_unfinished_message_gives_way_to_a_new_one(void) {
  SKWSMessageAssembler a(4096);
  const std::string m = delta(3000), next = delta(200);
  TEST_ASSERT_TRUE(feed(a, kText, false, m.substr(0, 1500)) ==
                   Result::kPartial);
  TEST_ASSERT_TRUE(feed(a, kText, true, next) == Result::kComplete);
  TEST_ASSERT_EQUAL_STRING(next.c_str(), a.message().c_str());
}

/// A new connection: the half message from the old one is gone.
void test_reset_forgets_a_message_in_progress(void) {
  SKWSMessageAssembler a(4096);
  const std::string m = delta(2500);
  const int len = static_cast<int>(m.size());
  TEST_ASSERT_TRUE(a.add(kText, true, len, 0, m.data(), 1024) ==
                   Result::kPartial);
  a.reset();
  TEST_ASSERT_TRUE(a.add(kText, true, len, 1024, m.data() + 1024, 1024) ==
                   Result::kDropped);
}

/// RFC 6455 allows a zero-length final fragment.
void test_empty_final_fragment_ends_the_message(void) {
  SKWSMessageAssembler a(4096);
  const std::string m = "{\"a\":1}";
  TEST_ASSERT_TRUE(feed(a, kText, false, m) == Result::kPartial);
  TEST_ASSERT_TRUE(a.add(kCont, true, 0, 0, "", 0) == Result::kComplete);
  TEST_ASSERT_EQUAL_STRING(m.c_str(), a.message().c_str());
}

/// Back-to-back large messages reuse the buffer rather than regrow it.
void test_buffer_is_kept_between_messages(void) {
  SKWSMessageAssembler a(4096);
  const std::string m1 = delta(3000), m2 = delta(3500);
  TEST_ASSERT_TRUE(feed(a, kText, true, m1) == Result::kComplete);
  const size_t capacity = a.message().capacity();
  TEST_ASSERT_TRUE(capacity >= m1.size());
  const int len = static_cast<int>(m2.size());
  TEST_ASSERT_TRUE(a.add(kText, true, len, 0, m2.data(), 10) ==
                   Result::kPartial);
  TEST_ASSERT_EQUAL(capacity, a.message().capacity());
}

/// Negative lengths or a null piece cannot come from the client, but would
/// corrupt the buffer if they did.
void test_bad_arguments_are_dropped(void) {
  SKWSMessageAssembler a(4096);
  TEST_ASSERT_TRUE(a.add(kText, true, 10, 0, nullptr, 5) == Result::kDropped);
  TEST_ASSERT_TRUE(a.add(kText, true, -1, 0, "x", 1) == Result::kDropped);
  TEST_ASSERT_TRUE(a.add(kText, true, 10, -3, "x", 1) == Result::kDropped);
  TEST_ASSERT_TRUE(feed(a, kText, true, "{}") == Result::kComplete);
}

/// The fin flag is read where the client's event has one, and only there.
void test_frame_end_follows_the_event_struct(void) {
  TEST_ASSERT_TRUE(ws_frame_end(EventWithFin{true, kText}) ==
                   WSFrameEnd::kLast);
  TEST_ASSERT_TRUE(ws_frame_end(EventWithFin{false, kText}) ==
                   WSFrameEnd::kMore);
  TEST_ASSERT_TRUE(ws_frame_end(EventWithoutFin{kText}) == kUnknown);
}

/// Stands in for fin on ESP-IDF 4.4, so a wrong answer splits or merges
/// messages.
void test_json_end_scanner(void) {
  TEST_ASSERT_TRUE(scans_complete("{\"a\":[1,{\"b\":2}]}"));
  TEST_ASSERT_FALSE(scans_complete("{\"a\":[1,{\"b\":2}]"));
  TEST_ASSERT_TRUE(scans_complete("  \r\n[1,2]\n"));
  TEST_ASSERT_FALSE(scans_complete(""));
  TEST_ASSERT_FALSE(scans_complete(" \n"));
  // Brackets and quotes inside strings do not count, split anywhere.
  const std::string tricky = "{\"p\":\"}]{[\\\"\\\\\",\"q\":\"x\\\\\"}";
  for (size_t piece = 1; piece <= tricky.size(); piece++) {
    TEST_ASSERT_TRUE(scans_complete(tricky, piece));
    TEST_ASSERT_FALSE(
        scans_complete(tricky.substr(0, tricky.size() - 1), piece));
  }
  TEST_ASSERT_TRUE(scans_complete("42"));
  TEST_ASSERT_FALSE(scans_complete("\"open"));
  TEST_ASSERT_TRUE(scans_complete("\"closed\""));
}

/// Without fin, the frame end is still the message end once the JSON has
/// closed: no wait for a continuation that will not come.
void test_unknown_fin_single_frame(void) {
  SKWSMessageAssembler a(4096);
  const std::string m = delta(2500);
  TEST_ASSERT_TRUE(feed(a, kText, kUnknown, m) == Result::kComplete);
  TEST_ASSERT_EQUAL_STRING(m.c_str(), a.message().c_str());
}

/// Without fin, an open object at a frame end means continuation frames
/// follow; control frames between them are skipped as with fin.
void test_unknown_fin_fragmented_message_is_joined(void) {
  SKWSMessageAssembler a(4096);
  const std::string m = delta(3000);
  TEST_ASSERT_TRUE(feed(a, kText, kUnknown, m.substr(0, 1200)) ==
                   Result::kPartial);
  TEST_ASSERT_TRUE(a.add(kPing, kUnknown, 0, 0, nullptr, 0) ==
                   Result::kIgnored);
  TEST_ASSERT_TRUE(feed(a, kCont, kUnknown, m.substr(1200, 1000)) ==
                   Result::kPartial);
  TEST_ASSERT_TRUE(feed(a, kCont, kUnknown, m.substr(2200)) ==
                   Result::kComplete);
  TEST_ASSERT_EQUAL_STRING(m.c_str(), a.message().c_str());
  TEST_ASSERT_TRUE(feed(a, kText, kUnknown, "{}") == Result::kComplete);
}

/// Without fin: the limit applies to the whole message, and no part of a
/// too-large one is handed on.
void test_unknown_fin_too_large_fragmented_message(void) {
  SKWSMessageAssembler a(4096);
  const std::string big = delta(9000), next = delta(100);
  TEST_ASSERT_TRUE(feed(a, kText, kUnknown, big.substr(0, 3000)) ==
                   Result::kPartial);
  TEST_ASSERT_TRUE(feed(a, kCont, kUnknown, big.substr(3000, 3000)) ==
                   Result::kPartial);
  TEST_ASSERT_TRUE(feed(a, kCont, kUnknown, big.substr(6000)) ==
                   Result::kDropped);
  TEST_ASSERT_TRUE(feed(a, kText, kUnknown, next) == Result::kComplete);
  TEST_ASSERT_EQUAL_STRING(next.c_str(), a.message().c_str());
}

/// Without fin, a broken message that never closes would otherwise hold
/// the assembler until the size limit: the next text frame replaces it.
void test_unknown_fin_unclosed_message_gives_way(void) {
  SKWSMessageAssembler a(4096);
  const std::string next = delta(200);
  TEST_ASSERT_TRUE(feed(a, kText, kUnknown, "{\"updates\":[") ==
                   Result::kPartial);
  TEST_ASSERT_TRUE(feed(a, kText, kUnknown, next) == Result::kComplete);
  TEST_ASSERT_EQUAL_STRING(next.c_str(), a.message().c_str());
}

/// Without fin, no split of a valid message hands on part of it: a proper
/// prefix of a JSON object never closes it, wherever the server cuts it.
void test_unknown_fin_no_split_point_completes_early(void) {
  const std::string m =
      "{\"context\":\"vessels.urn:mrn:imo:mmsi:230000000\",\"updates\":[{"
      "\"source\":{\"label\":\"n2k {gw}\",\"src\":\"115\"},\"values\":[{"
      "\"path\":\"notifications.x\",\"value\":{\"message\":\"Say \\\"}]\\\" "
      "then \\\"x\\\" "
      "\\\\\",\"state\":\"normal\"}},{\"path\":\"a.b\",\"value\":"
      "[1.5,-2e3,true,null]}]}]}";
  for (size_t cut = 1; cut < m.size(); cut++) {
    SKWSMessageAssembler a(4096);
    TEST_ASSERT_TRUE(feed(a, kText, kUnknown, m.substr(0, cut)) ==
                     Result::kPartial);
    TEST_ASSERT_TRUE(feed(a, kCont, kUnknown, m.substr(cut)) ==
                     Result::kComplete);
    TEST_ASSERT_EQUAL_STRING(m.c_str(), a.message().c_str());
  }
}

/// Unity hook; each test makes its own assembler.
void setUp(void) {}
/// Unity hook; nothing to clean up.
void tearDown(void) {}

/// Host test runner (`pio test -e native`).
int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_whole_message_in_one_piece);
  RUN_TEST(test_frame_larger_than_the_buffer_is_joined);
  RUN_TEST(test_uneven_pieces_from_tcp_are_joined);
  RUN_TEST(test_fragmented_message_with_a_ping_between);
  RUN_TEST(test_messages_one_after_another);
  RUN_TEST(test_too_large_a_message_is_dropped_once);
  RUN_TEST(test_message_of_exactly_the_limit_is_kept);
  RUN_TEST(test_a_piece_out_of_place_loses_only_that_message);
  RUN_TEST(test_continuation_without_a_start_is_dropped);
  RUN_TEST(test_an_unfinished_message_gives_way_to_a_new_one);
  RUN_TEST(test_reset_forgets_a_message_in_progress);
  RUN_TEST(test_empty_final_fragment_ends_the_message);
  RUN_TEST(test_buffer_is_kept_between_messages);
  RUN_TEST(test_bad_arguments_are_dropped);
  RUN_TEST(test_frame_end_follows_the_event_struct);
  RUN_TEST(test_json_end_scanner);
  RUN_TEST(test_unknown_fin_single_frame);
  RUN_TEST(test_unknown_fin_fragmented_message_is_joined);
  RUN_TEST(test_unknown_fin_too_large_fragmented_message);
  RUN_TEST(test_unknown_fin_unclosed_message_gives_way);
  RUN_TEST(test_unknown_fin_no_split_point_completes_early);
  return UNITY_END();
}
