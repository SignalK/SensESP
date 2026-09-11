#include "wifi_watchdog.h"

#include <Arduino.h>

#include "sensesp.h"

namespace sensesp {

namespace {

constexpr unsigned int kPollIntervalMs = 1000;
constexpr unsigned int kRestartDelayMs = 1000;

}  // namespace

WiFiWatchdog::WiFiWatchdog(
    std::shared_ptr<ValueProducer<NetworkState>> network_state, int timeout_s,
    const String& config_path)
    : FileSystemSaveable(config_path),
      timeout_s_(timeout_s),
      state_consumer_([this](NetworkState state) { on_network_state(state); }) {
  load();
  network_state->connect_to(&state_consumer_);
  event_loop()->onRepeat(kPollIntervalMs, [this]() { poll(); });
}

void WiFiWatchdog::on_network_state(NetworkState state) {
  policy_.set_link_state(wifi_watchdog_link_from_state(state), millis());
}

void WiFiWatchdog::poll() {
  if (restart_pending_) {
    return;
  }
  const uint32_t timeout_ms = static_cast<uint32_t>(timeout_s_) * 1000;
  if (policy_.restart_due(millis(), timeout_ms)) {
    ESP_LOGW(__FILENAME__, "Network down for more than %d s; restarting.",
             timeout_s_);
    restart_pending_ = true;
    event_loop()->onDelay(kRestartDelayMs, []() { ESP.restart(); });
  }
}

bool WiFiWatchdog::to_json(JsonObject& root) {
  root["timeout_s"] = timeout_s_;
  return true;
}

bool WiFiWatchdog::from_json(const JsonObject& config) {
  if (!config["timeout_s"].is<int>()) {
    return false;
  }
  const int timeout_s = config["timeout_s"].as<int>();
  if (!is_valid_timeout_s(timeout_s)) {
    return false;
  }
  timeout_s_ = timeout_s;
  return true;
}

const String ConfigSchema(const WiFiWatchdog& obj) {
  String schema = R"json({"type":"object","properties":{"timeout_s":{"title":"Timeout (s)","description":"Restart the device after the network has been down for this long. Choose a value that outlasts an access point reboot.","type":"integer","minimum":1,"maximum":{{max}}}}})json";
  schema.replace("{{max}}", String(WiFiWatchdog::kMaxTimeoutS));
  return schema;
}

}  // namespace sensesp
