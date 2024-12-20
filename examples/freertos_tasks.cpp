/**
 * @brief Example and test program for using FreeRTOS tasks.
 *
 */

#include "sensesp/controllers/system_status_controller.h"
#include "sensesp/net/discovery.h"
#include "sensesp/net/http_server.h"
#include "sensesp/net/networking.h"
#include "sensesp/net/web/app_command_handler.h"
#include "sensesp/net/web/base_command_handler.h"
#include "sensesp/net/web/config_handler.h"
#include "sensesp/net/web/static_file_handler.h"
#include "sensesp/sensors/digital_input.h"
#include "sensesp/signalk/signalk_delta_queue.h"
#include "sensesp/signalk/signalk_output.h"
#include "sensesp/signalk/signalk_value_listener.h"
#include "sensesp/signalk/signalk_ws_client.h"
#include "sensesp/system/lambda_consumer.h"
#include "sensesp/system/system_status_led.h"
#include "sensesp_minimal_app_builder.h"

using namespace sensesp;

const int kTestOutputPin = GPIO_NUM_18;
// repetition interval in ms; corresponds to 1000/(2*5)=100 Hz
const int kTestOutputInterval = 410;

const uint8_t kDigitalInputPin = 15;

void ToggleTestOutputPin(void *parameter) {
  while (true) {
    digitalWrite(kTestOutputPin, !digitalRead(kTestOutputPin));
    delay(kTestOutputInterval);
  }
}

// The setup function performs one-time application initialization.
void setup() {
  SetupLogging();

  SensESPMinimalAppBuilder builder;
  auto sensesp_app = builder.set_hostname("async")->get_app();

  auto networking = std::make_shared<Networking>("/system/networking", "", "");
  auto http_server = std::make_shared<HTTPServer>();

  // Add the default HTTP server response handlers
  add_static_file_handlers(http_server);
  add_base_app_http_command_handlers(http_server);
  add_app_http_command_handlers(http_server, networking);
  add_config_handlers(http_server);

  // create the SK delta object
  auto sk_delta_queue_ = std::make_shared<SKDeltaQueue>();

  // create the websocket client
  auto ws_client_ =
      std::make_shared<SKWSClient>("/system/sk", sk_delta_queue_, "", 0);

  auto output_consumer = std::make_shared<LambdaConsumer<SKWSConnectionState>>(
      [](SKWSConnectionState input) {
        ESP_LOGD("Example", "SKWSConnectionState: %d", input);
      });

  ws_client_->connect_to(output_consumer);

  // create the MDNS discovery object
  auto mdns_discovery_ = std::make_shared<MDNSDiscovery>();

  // create a system status controller and a led blinker

  auto system_status_controller = std::make_shared<SystemStatusController>();
  auto system_status_led = std::make_shared<SystemStatusLed>(LED_BUILTIN);

  system_status_controller->connect_to(
      system_status_led->get_system_status_consumer());
  ws_client_->get_delta_tx_count_producer().connect_to(
      system_status_led->get_delta_tx_count_consumer());

  // create a new task for toggling the output pin

  pinMode(kTestOutputPin, OUTPUT);
  xTaskCreate(ToggleTestOutputPin, "toggler", 2048, NULL, 1, NULL);

  // listen to the changes on the digital input pin

  auto digin = new DigitalInputChange(kDigitalInputPin, INPUT_PULLUP, CHANGE);

  digin->connect_to(new LambdaConsumer<bool>([](bool input) {
    ESP_LOGD("Example", "(%d ms) Digital input changed to %d", millis(), input);
  }));

  // connect digin to the SK delta queue

  const char *sk_path = "environment.bool.pin15";
  digin->connect_to(new SKOutputFloat(sk_path, ""));

  // create a new SKListener for navigation.headingMagnetic

  auto hdg = new SKValueListener<float>("navigation.headingMagnetic");
  hdg->connect_to(new LambdaConsumer<float>(
      [](float input) { ESP_LOGD("Example", "Heading: %f", input); }));

  // print out free heap
  event_loop()->onRepeat(
      2000, []() { ESP_LOGD("Example", "Free heap: %d", ESP.getFreeHeap()); });
}

void loop() { event_loop()->tick(); }
