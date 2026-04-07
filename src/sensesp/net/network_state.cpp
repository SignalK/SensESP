#include "sensesp.h"

#include "network_state.h"

#include <Network.h>

namespace sensesp {

namespace {

/**
 * @brief Helper that re-emits a NetworkState through the SensESP event
 * loop instead of from the calling task.
 *
 * Network event callbacks are invoked from the LwIP / system event task.
 * Doing the emission directly would expose downstream consumers to
 * cross-task races (the SensESP reactive layer assumes single-threaded
 * execution on the main loop). Hopping through `event_loop()->onDelay(0,
 * …)` defers the work to the next loop iteration on the main task.
 */
void defer_emit(NetworkStateProducer* producer, NetworkState state) {
  event_loop()->onDelay(0, [producer, state]() { producer->emit(state); });
}

}  // namespace

NetworkStateProducer::NetworkStateProducer() {
  this->output_ = kNetworkNoConnection;

  // ----- WiFi events ------------------------------------------------------
  wifi_got_ip_handle_ = Network.onEvent(
      [this](arduino_event_id_t, arduino_event_info_t) {
        ESP_LOGI("net_state", "WiFi station got IP");
        defer_emit(this, kNetworkConnected);
      },
      ARDUINO_EVENT_WIFI_STA_GOT_IP);

  wifi_disconnected_handle_ = Network.onEvent(
      [this](arduino_event_id_t, arduino_event_info_t) {
        ESP_LOGI("net_state", "WiFi station disconnected");
        defer_emit(this, kNetworkDisconnected);
      },
      ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  wifi_ap_start_handle_ = Network.onEvent(
      [this](arduino_event_id_t, arduino_event_info_t) {
        ESP_LOGI("net_state", "WiFi soft-AP started");
        defer_emit(this, kNetworkAPMode);
      },
      ARDUINO_EVENT_WIFI_AP_START);

  wifi_ap_stop_handle_ = Network.onEvent(
      [this](arduino_event_id_t, arduino_event_info_t) {
        ESP_LOGI("net_state", "WiFi soft-AP stopped");
        defer_emit(this, kNetworkDisconnected);
      },
      ARDUINO_EVENT_WIFI_AP_STOP);

  // ----- Ethernet events --------------------------------------------------
  eth_got_ip_handle_ = Network.onEvent(
      [this](arduino_event_id_t, arduino_event_info_t) {
        ESP_LOGI("net_state", "Ethernet got IP");
        defer_emit(this, kNetworkConnected);
      },
      ARDUINO_EVENT_ETH_GOT_IP);

  eth_lost_ip_handle_ = Network.onEvent(
      [this](arduino_event_id_t, arduino_event_info_t) {
        ESP_LOGI("net_state", "Ethernet lost IP");
        defer_emit(this, kNetworkDisconnected);
      },
      ARDUINO_EVENT_ETH_LOST_IP);

  eth_disconnected_handle_ = Network.onEvent(
      [this](arduino_event_id_t, arduino_event_info_t) {
        ESP_LOGI("net_state", "Ethernet PHY disconnected");
        defer_emit(this, kNetworkDisconnected);
      },
      ARDUINO_EVENT_ETH_DISCONNECTED);

  // Re-emit the current (initial) state once the event loop is running so
  // late subscribers (added in SensESPApp::setup) see the latest value.
  event_loop()->onDelay(0, [this]() { this->emit(this->output_); });
}

NetworkStateProducer::~NetworkStateProducer() {
  Network.removeEvent(wifi_got_ip_handle_);
  Network.removeEvent(wifi_disconnected_handle_);
  Network.removeEvent(wifi_ap_start_handle_);
  Network.removeEvent(wifi_ap_stop_handle_);
  Network.removeEvent(eth_got_ip_handle_);
  Network.removeEvent(eth_lost_ip_handle_);
  Network.removeEvent(eth_disconnected_handle_);
}

}  // namespace sensesp
