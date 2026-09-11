#include "sensesp.h"

#include "wifi_watchdog.h"

#include <Arduino.h>

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
      network_state_(network_state) {
  load();
  state_observer_id_ = network_state_->attach(
      [this]() { on_network_state(network_state_->get()); });
  poll_event_ = event_loop()->onRepeat(kPollIntervalMs, [this]() { poll(); });
}

WiFiWatchdog::~WiFiWatchdog() {
  network_state_->detach(state_observer_id_);
  poll_event_->remove(event_loop());
  if (restart_event_ != nullptr) {
    restart_event_->remove(event_loop());
  }
}

void WiFiWatchdog::on_network_state(NetworkState state) {
  const WiFiWatchdogLink link = wifi_watchdog_link_from_state(state);
  policy_.set_link_state(link, millis());
  if (link == WiFiWatchdogLink::kUp && restart_event_ != nullptr) {
    ESP_LOGI(__FILENAME__, "Network is back; cancelling the restart.");
    restart_event_->remove(event_loop());
    restart_event_ = nullptr;
  }
}

void WiFiWatchdog::poll() {
  if (restart_event_ != nullptr) {
    return;
  }
  if (policy_.restart_due(millis(), timeout_ms())) {
    ESP_LOGW(__FILENAME__, "Network down for more than %d s; restarting.",
             timeout_s_);
    restart_event_ = event_loop()->onDelay(kRestartDelayMs, [this]() {
      restart_event_ = nullptr;
      // The timeout may have been raised from the web UI meanwhile.
      if (policy_.restart_due(millis(), timeout_ms())) {
        ESP.restart();
      }
    });
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
  String schema = R"json({"type":"object","properties":{"timeout_s":{"title":"Timeout (s)","description":"Restart the device after the network has been down for this long. Choose a value that outlasts an access point reboot (1 to {{max}} seconds).","type":"integer","minimum":1,"maximum":{{max}}}}})json";
  schema.replace("{{max}}", String(WiFiWatchdog::kMaxTimeoutS));
  return schema;
}

}  // namespace sensesp
