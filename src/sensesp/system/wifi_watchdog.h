#ifndef SENSESP_SRC_SENSESP_SYSTEM_WIFI_WATCHDOG_H_
#define SENSESP_SRC_SENSESP_SYSTEM_WIFI_WATCHDOG_H_

#include <memory>

#include "sensesp/net/network_state.h"
#include "sensesp/system/lambda_consumer.h"
#include "sensesp/system/saveable.h"
#include "sensesp/system/valueproducer.h"
#include "sensesp/system/wifi_watchdog_policy.h"

namespace sensesp {

/**
 * @brief Restart the device after a prolonged network outage.
 *
 * Consumes NetworkState from the app's NetworkStateProducer and polls a
 * WiFiWatchdogPolicy once a second. The watchdog arms on the first
 * station or Ethernet connection after boot, so a device that starts
 * without a reachable access point is left running. The soft-AP and the
 * captive portal neither arm the watchdog nor count as an outage. Once
 * armed, a continuous outage of at least the configured timeout restarts
 * the device.
 *
 * The timeout is persisted under the config path and editable from the web
 * UI. It is read on every poll, so a change takes effect without a restart.
 * A persisted value takes precedence over the constructor argument on later
 * boots.
 *
 * @param network_state Producer of the link state to watch.
 * @param timeout_s Initial outage duration in seconds before restarting.
 *   Must satisfy is_valid_timeout_s(); the caller validates.
 * @param config_path Path under which the timeout is persisted and shown in
 *   the web UI.
 */
class WiFiWatchdog : public FileSystemSaveable {
 public:
  // Bound well inside what 32-bit millisecond arithmetic can hold.
  static constexpr int kMaxTimeoutS = 7 * 24 * 3600;

  static bool is_valid_timeout_s(int timeout_s) {
    return timeout_s >= 1 && timeout_s <= kMaxTimeoutS;
  }

  WiFiWatchdog(std::shared_ptr<ValueProducer<NetworkState>> network_state,
               int timeout_s,
               const String& config_path = "/System/WiFi Watchdog");

  virtual bool to_json(JsonObject& root) override;
  virtual bool from_json(const JsonObject& config) override;

 protected:
  void on_network_state(NetworkState state);
  void poll();

  int timeout_s_;
  bool restart_pending_ = false;
  WiFiWatchdogPolicy policy_;
  LambdaConsumer<NetworkState> state_consumer_;
};

const String ConfigSchema(const WiFiWatchdog& obj);

}  // namespace sensesp

#endif  // SENSESP_SRC_SENSESP_SYSTEM_WIFI_WATCHDOG_H_
