#ifndef SENSESP_SRC_SENSESP_SIGNALK_SIGNALK_WS_CONNECT_DUE_H_
#define SENSESP_SRC_SENSESP_SIGNALK_SIGNALK_WS_CONNECT_DUE_H_

#include <cstdint>

namespace sensesp {

/**
 * @brief True if the next Signal K connection attempt is due.
 *
 * Pure and dependency-free (no Arduino) so it can be unit-tested on the host.
 *
 * Compares elapsed time, not absolute timestamps: millis() wraps after about
 * 49.7 days, and the time of the last attempt is kept for as long as the
 * connection lasts. Unsigned subtraction gives the right elapsed time across
 * the wrap, and a signed comparison would fail once a connection outlasts
 * half the range (about 24.8 days).
 */
inline bool sk_connect_due(uint32_t now_ms, uint32_t last_attempt_ms,
                           uint32_t wait_ms) {
  return now_ms - last_attempt_ms >= wait_ms;
}

}  // namespace sensesp

#endif  // SENSESP_SRC_SENSESP_SIGNALK_SIGNALK_WS_CONNECT_DUE_H_
