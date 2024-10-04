/**
 * @brief Example and test program for using FreeRTOS tasks.
 *
 */

#include "sensesp/controllers/system_status_controller.h"
#include "sensesp/net/discovery.h"
#include "sensesp/net/http_server.h"
#include "sensesp/net/networking.h"
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
  SensESPMinimalApp *sensesp_app = builder.set_hostname("async")->get_app();

  auto *networking = new Networking("/system/networking", "", "");
  auto *http_server = new HTTPServer();

  // create the SK delta object
  auto sk_delta_queue_ = new SKDeltaQueue();

  // create the websocket client
  auto ws_client_ = new SKWSClient("/system/sk", sk_delta_queue_, "", 0);

  ws_client_->connect_to(
      new LambdaConsumer<SKWSConnectionState>([](SKWSConnectionState input) {
        ESP_LOGD("Example", "SKWSConnectionState: %d", input);
      }));

  // create the MDNS discovery object
  auto mdns_discovery_ = new MDNSDiscovery();

  // create a system status controller and a led blinker

  auto *system_status_controller = new SystemStatusController();
  auto *system_status_led = new SystemStatusLed(LED_BUILTIN);

  system_status_controller->connect_to(system_status_led);
  ws_client_->get_delta_tx_count_producer().connect_to(system_status_led);

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
