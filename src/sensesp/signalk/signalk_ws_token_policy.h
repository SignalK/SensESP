#ifndef SENSESP_SRC_SENSESP_SIGNALK_SIGNALK_WS_TOKEN_POLICY_H_
#define SENSESP_SRC_SENSESP_SIGNALK_SIGNALK_WS_TOKEN_POLICY_H_

namespace sensesp {

// HTTP status returned on a WebSocket upgrade when the Signal K server rejects
// the auth token.
static constexpr int kHttpUnauthorized = 401;

/**
 * @brief Decide whether a failed WebSocket upgrade should clear the auth token.
 *
 * A 401 on the upgrade means the server rejected the presented token, and the
 * device must re-request access to recover; the caller acts on it only when a
 * token was actually sent. The decision is transport-independent: over
 * plaintext the status is unauthenticated, so anything that can answer for the
 * server (an on-path attacker, or any same-network device when mDNS discovery
 * is enabled) can inject a 401 to force a token wipe and a manual re-approval.
 * But gating on TLS leaves a plaintext deployment with no recovery route at
 * all, locked out indefinitely on a stale token. Recovering on its own is the
 * better failure mode. TLS deployments with a pinned TOFU anchor are
 * unaffected, since their 401 is authenticated; with TOFU disabled or before
 * an anchor is pinned, the status is as spoofable as plaintext.
 */
inline bool should_clear_token_on_status(int handshake_status) {
  return handshake_status == kHttpUnauthorized;
}

}  // namespace sensesp

#endif
