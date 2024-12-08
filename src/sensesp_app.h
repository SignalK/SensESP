#ifndef SENSESP_APP_H_
#define SENSESP_APP_H_

#include "sensesp/controllers/system_status_controller.h"
#include "sensesp/net/discovery.h"
#include "sensesp/net/http_server.h"
#include "sensesp/net/networking.h"
#include "sensesp/net/ota.h"
#include "sensesp/net/web/app_command_handler.h"
#include "sensesp/net/web/base_command_handler.h"
#include "sensesp/net/web/config_handler.h"
#include "sensesp/net/web/static_file_handler.h"
#include "sensesp/sensesp_version.h"
#include "sensesp/sensors/sensor.h"
#include "sensesp/signalk/signalk_delta_queue.h"
#include "sensesp/signalk/signalk_ws_client.h"
#include "sensesp/system/button.h"
#include "sensesp/system/system_status_led.h"
#include "sensesp/ui/status_page_item.h"
#include "sensesp_base_app.h"

namespace sensesp {

class SensESPApp;
// I'd rather not have this global variable here but many legacy examples
// access it. Use SensESPApp::get() instead.
extern std::shared_ptr<SensESPApp> sensesp_app;

/**
 * The default SensESP application object with networking and Signal K
 * communication.
 * @see SensESPAppBuilder
 */
class SensESPApp : public SensESPBaseApp {
 public:
  /**
   * Singletons should not be cloneable
   */
  SensESPApp(SensESPApp& other) = delete;

  /**
   * Singletons should not be assignable
   */
  void operator=(const SensESPApp&) = delete;

  /**
   * @brief Get the singleton instance of the SensESPApp
   */
  static std::shared_ptr<SensESPApp> get() {
    if (instance_ == nullptr) {
      instance_ = std::shared_ptr<SensESPApp>(new SensESPApp());
    }
    return std::static_pointer_cast<SensESPApp>(instance_);
  }

  virtual bool destroy() override {
    bool outside_users = instance_.use_count() > 2;
    if (outside_users) {
      ESP_LOGW(
          __FILENAME__,
          "SensESPApp instance has active references and won't be properly "
          "destroyed.");
    }
    instance_ = nullptr;
    // Also destroy the global pointer
    sensesp_app = nullptr;
    return !outside_users;
  }

  // getters for internal members
  std::shared_ptr<SKDeltaQueue> get_sk_delta() { return this->sk_delta_queue_; }
  std::shared_ptr<SystemStatusController> get_system_status_controller() {
    return this->system_status_controller_;
  }
  std::shared_ptr<Networking>& get_networking() { return this->networking_; }
  std::shared_ptr<SKWSClient> get_ws_client() { return this->ws_client_; }

 protected:
  /**
   * @brief SensESPApp constructor
   *
   * Note that the constructor is protected, so SensESPApp should only
   * be instantiated using SensESPAppBuilder.
   *
   */
  SensESPApp() : SensESPBaseApp() {}

  // setters for all constructor arguments

  const SensESPApp* set_hostname(String hostname) {
    this->SensESPBaseApp::set_hostname(hostname);
    return this;
  }
  const SensESPApp* set_ssid(String ssid) {
    this->ssid_ = ssid;
    return this;
  }
  const SensESPApp* set_wifi_password(String wifi_password) {
    this->wifi_client_password_ = wifi_password;
    return this;
  }
  const SensESPApp* set_ap_ssid(const String& ssid) {
    this->ap_ssid_ = ssid;
    return this;
  }
  const SensESPApp* set_ap_password(const String& password) {
    this->ap_password_ = password;
    return this;
  }
  const SensESPApp* set_sk_server_address(String sk_server_address) {
    this->sk_server_address_ = sk_server_address;
    return this;
  }
  const SensESPApp* set_sk_server_port(uint16_t sk_server_port) {
    this->sk_server_port_ = sk_server_port;
    return this;
  }
  const SensESPApp* set_system_status_led(
      std::shared_ptr<SystemStatusLed>& system_status_led) {
    this->system_status_led_ = system_status_led;
    return this;
  }
  const SensESPApp* set_admin_user(const char* username, const char* password) {
    this->http_server_->set_auth_credentials(username, password, true);
    return this;
  }
  const SensESPApp* enable_ota(const char* password) {
    ota_password_ = password;
    return this;
  }
  const SensESPApp* set_button_pin(int pin) {
    button_gpio_pin_ = pin;
    return this;
  }

  /**
   * @brief Perform initialization of SensESPApp once builder configuration is
   * done.
   *
   * This should be only called from the builder!
   *
   */
  void setup() {
    // call the parent setup()
    SensESPBaseApp::setup();

    ap_ssid_ = SensESPBaseApp::get_hostname();

    // create the networking object
    networking_ = std::make_shared<Networking>("/System/WiFi Settings", ssid_,
                                               wifi_client_password_, ap_ssid_,
                                               ap_password_);

    ConfigItem(networking_);

    if (ota_password_ != nullptr) {
      // create the OTA object
      ota_ = std::make_shared<OTA>(ota_password_);
    }

    bool captive_portal_enabled = networking_->is_captive_portal_enabled();

    // create the HTTP server
    this->http_server_ = std::make_shared<HTTPServer>();
    this->http_server_->set_captive_portal(captive_portal_enabled);

    // Add the default HTTP server response handlers
    add_static_file_handlers(this->http_server_);
    add_base_app_http_command_handlers(this->http_server_);
    add_app_http_command_handlers(this->http_server_);
    add_config_handlers(this->http_server_);

    ConfigItem(this->http_server_);

    // create the SK delta object
    sk_delta_queue_ = std::make_shared<SKDeltaQueue>();

    // create the websocket client
    bool const use_mdns = sk_server_address_ == "";
    this->ws_client_ = std::make_shared<SKWSClient>(
        "/System/Signal K Settings", sk_delta_queue_, sk_server_address_,
        sk_server_port_, use_mdns);

    ConfigItem(this->ws_client_);

    // connect the system status controller
    this->networking_->get_wifi_state_producer()->connect_to(
        &system_status_controller_->get_wifi_state_consumer());
    this->ws_client_->connect_to(
        &system_status_controller_->get_ws_connection_state_consumer());

    // create the MDNS discovery object
    mdns_discovery_ = std::make_shared<MDNSDiscovery>();

    // create a system status led and connect it

    if (system_status_led_ == nullptr) {
#ifdef PIN_NEOPIXEL
      system_status_led_ = std::make_shared<RGBSystemStatusLed>(PIN_NEOPIXEL);
#elif defined(LED_BUILTIN)
      system_status_led_ = std::make_shared<SystemStatusLed>(LED_BUILTIN);
#endif
    }
    if (system_status_led_ != nullptr) {
      this->system_status_controller_->connect_to(
          system_status_led_->system_status_consumer_);
      this->ws_client_->get_delta_tx_count_producer().connect_to(
          system_status_led_->get_delta_tx_count_consumer());
    }

    // create the button handler
    if (button_gpio_pin_ != -1) {
      button_handler_ = std::make_shared<ButtonHandler>(button_gpio_pin_);
    }

    // connect status page items
    connect_status_page_items();
  }

  // Collect metrics for the status page
  void connect_status_page_items() {
    this->hostname_->connect_to(&this->hostname_ui_output_);
    this->event_loop_->onRepeat(4999, [this]() {
      wifi_ssid_ui_output_.set(WiFi.SSID());
      free_memory_ui_output_.set(ESP.getFreeHeap());
      wifi_rssi_ui_output_.set(WiFi.RSSI());

      // Uptime
      uptime_ui_output_.set(millis() / 1000);

      // Event loop queue sizes
      int event_loop_queue_size = event_loop_->getEventQueueSize();
      int event_loop_timed_queue_size = event_loop_->getTimedEventQueueSize();
      int event_loop_untimed_queue_size =
          event_loop_->getUntimedEventQueueSize();

      // Total tick count
      uint64_t current_tick_count = event_loop_->getTickCount();
      total_tick_count_ui_output_.set(current_tick_count);

      // Event counts
      uint64_t current_event_count = event_loop_->getEventCount();
      uint64_t current_timed_event_count = event_loop_->getTimedEventCount();
      uint64_t current_untimed_event_count =
          event_loop_->getUntimedEventCount();
      event_count_ui_output_.set(current_event_count);
      timed_event_count_ui_output_.set(current_timed_event_count);
      untimed_event_count_ui_output_.set(current_untimed_event_count);

      // Ticks and events per second during last interval
      static uint64_t last_tick_count = 0;
      static uint64_t last_event_count = 0;
      static uint64_t last_timed_event_count = 0;
      static uint64_t last_untimed_event_count = 0;
      static uint64_t last_millis = 0;
      uint64_t current_millis = millis();
      float interval_seconds = (current_millis - last_millis) / 1000.0;

      uint64_t ticks_diff = current_tick_count - last_tick_count;
      uint64_t events_diff = current_event_count - last_event_count;
      uint64_t timed_events_diff =
          current_timed_event_count - last_timed_event_count;
      uint64_t untimed_events_diff =
          current_untimed_event_count - last_untimed_event_count;

      // Set outputs
      event_loop_queue_size_ui_output_.set(event_loop_queue_size);
      event_loop_timed_queue_ui_output_.set(event_loop_timed_queue_size);
      event_loop_untimed_queue_ui_output_.set(event_loop_untimed_queue_size);
      event_loop_interrupt_queue_ui_output_.set(
          event_loop_->getISREventQueueSize());

      ticks_per_second_ui_output_.set(int(ticks_diff / interval_seconds));
      events_per_second_ui_output_.set(int(events_diff / interval_seconds));
      timed_events_per_second_ui_output_.set(
          int(timed_events_diff / interval_seconds));
      untimed_events_per_second_ui_output_.set(
          int(untimed_events_diff / interval_seconds));

      // Update last values
      last_tick_count = current_tick_count;
      last_event_count = current_event_count;
      last_timed_event_count = current_timed_event_count;
      last_untimed_event_count = current_untimed_event_count;
      last_millis = current_millis;

      sk_server_address_ui_output_.set(ws_client_->get_server_address());
      sk_server_port_ui_output_.set(ws_client_->get_server_port());
      sk_server_connection_ui_output_.set(ws_client_->get_connection_status());
    });
    ws_client_->get_delta_tx_count_producer().connect_to(
        &delta_tx_count_ui_output_);
    ws_client_->get_delta_rx_count_producer().connect_to(
        &delta_rx_count_ui_output_);
  }

  String ssid_ = "";
  String wifi_client_password_ = "";
  String sk_server_address_ = "";
  uint16_t sk_server_port_ = 0;
  String ap_ssid_ = "";
  String ap_password_ = "thisisfine";
  const char* ota_password_ = nullptr;

  std::shared_ptr<MDNSDiscovery> mdns_discovery_;
  std::shared_ptr<HTTPServer> http_server_;

  std::shared_ptr<BaseSystemStatusLed> system_status_led_;
  std::shared_ptr<SystemStatusController> system_status_controller_ =
      std::make_shared<SystemStatusController>();
  int button_gpio_pin_ = SENSESP_BUTTON_PIN;
  std::shared_ptr<ButtonHandler> button_handler_;

  std::shared_ptr<Networking> networking_;

  std::shared_ptr<OTA> ota_;
  std::shared_ptr<SKDeltaQueue> sk_delta_queue_;
  std::shared_ptr<SKWSClient> ws_client_;

  StatusPageItem<int> free_memory_ui_output_{"Free memory (bytes)", 0, "System",
                                             1000};
  StatusPageItem<int> uptime_ui_output_{"Uptime (s)", 0, "System", 1100};

  StatusPageItem<String> hostname_ui_output_{"Hostname", "", "Network", 1200};
  StatusPageItem<String> mac_address_ui_output_{
      "MAC Address", WiFi.macAddress(), "Network", 1300};
  StatusPageItem<String> wifi_ssid_ui_output_{"SSID", "", "Network", 1400};

  StatusPageItem<int8_t> wifi_rssi_ui_output_{"WiFi signal strength (dB)", -128,
                                              "Network", 1500};

  StatusPageItem<String> sk_server_address_ui_output_{"Signal K server address",
                                                      "", "Signal K", 1600};
  StatusPageItem<uint16_t> sk_server_port_ui_output_{"Signal K server port", 0,
                                                     "Signal K", 1700};
  StatusPageItem<String> sk_server_connection_ui_output_{"SK connection status",
                                                         "", "Signal K", 1800};
  StatusPageItem<int> delta_tx_count_ui_output_{"SK Delta TX count", 0,
                                                "Signal K", 1900};
  StatusPageItem<int> delta_rx_count_ui_output_{"SK Delta RX count", 0,
                                                "Signal K", 2000};

  StatusPageItem<int> event_loop_queue_size_ui_output_{
      "Event Loop queue size", 0, "Event Loop Queues", 2100};
  StatusPageItem<int> event_loop_timed_queue_ui_output_{
      "Event Loop timed queue size", 0, "Event Loop Queues", 2200};
  StatusPageItem<int> event_loop_untimed_queue_ui_output_{
      "Event Loop untimed queue size", 0, "Event Loop Queues", 2300};
  StatusPageItem<int> event_loop_interrupt_queue_ui_output_{
      "Event Loop interrupt queue size", 0, "Event Loop Queues", 2400};

  StatusPageItem<uint64_t> total_tick_count_ui_output_{
      "Total ticks processed", 0, "Event Loop Lifetime", 2500};
  StatusPageItem<uint64_t> event_count_ui_output_{"Events processed", 0,
                                                  "Event Loop Lifetime", 2600};
  StatusPageItem<uint64_t> timed_event_count_ui_output_{
      "Timed events processed", 0, "Event Loop Lifetime", 2700};
  StatusPageItem<uint64_t> untimed_event_count_ui_output_{
      "Untimed events processed", 0, "Event Loop Lifetime", 2800};
  StatusPageItem<float> ticks_per_second_ui_output_{
      "Ticks per second", 0, "Event Loop Performance", 2900};
  StatusPageItem<float> events_per_second_ui_output_{
      "Events per second", 0, "Event Loop Performance", 3000};
  StatusPageItem<float> timed_events_per_second_ui_output_{
      "Timed events per second", 0, "Event Loop Performance", 3100};
  StatusPageItem<float> untimed_events_per_second_ui_output_{
      "Untimed events per second", 0, "Event Loop Performance", 3200};

  StatusPageItem<String> sensesp_version_ui_output_{
      "SenseESP version", kSensESPVersion, "Software", 3300};
  StatusPageItem<String> build_info_ui_output_{
      "Build date", __DATE__ " " __TIME__, "Software", 3400};

  // Placeholders for system status sensors in case they are created
  std::shared_ptr<ValueProducer<float>> system_hz_sensor_;
  std::shared_ptr<ValueProducer<uint32_t>> free_mem_sensor_;
  std::shared_ptr<ValueProducer<float>> uptime_sensor_;
  std::shared_ptr<ValueProducer<String>> ip_address_sensor_;
  std::shared_ptr<ValueProducer<int>> wifi_signal_sensor_;

  friend class WebServer;
  friend class SensESPAppBuilder;
};

}  // namespace sensesp

#endif
