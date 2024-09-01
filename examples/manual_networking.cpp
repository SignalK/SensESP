#include <math.h>

#include "sensesp/net/http_server.h"
#include "sensesp/net/networking.h"
#include "sensesp/sensors/digital_input.h"
#include "sensesp/system/lambda_consumer.h"
#include "sensesp/transforms/linear.h"
#include "sensesp/transforms/typecast.h"
#include "sensesp_minimal_app_builder.h"
#include "sensesp/signalk/signalk_output.h"

using namespace sensesp;

const unsigned int read_delay = 500;

const uint8_t input_pin1 = 0;

// This is a sample program to demonstrate how to instantiate a
// SensESPMinimalApp application and setup networking manually.
//
// The program reacts to changes on GPIO pin 0 and prints the value to the
// serial console.

void setup() {
  SetupLogging();

  SensESPMinimalAppBuilder builder;
  auto sensesp_app = builder.set_hostname("counter-test")->get_app();

  // manually create Networking and HTTPServer objects to enable
  // the HTTP configuration interface

  WiFi.mode(WIFI_STA);
  WiFi.begin("Hat Labs Sensors", "kanneluuri2406");

  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");

  ESP_LOGD("Example", "Connected to WiFi. IP address: %s", WiFi.localIP().toString().c_str());

  WiFi.setHostname(SensESPBaseApp::get_hostname().c_str());

  auto* http_server = new HTTPServer();

  auto* digin = new DigitalInputChange(input_pin1, INPUT, CHANGE);

  digin->connect_to(new LambdaConsumer<bool>([](bool input) {
    Serial.printf("millis: %d\n", millis());
    Serial.printf("Digin: %d\n", input);
  }));

  digin->connect_to(new SKOutputBool("electrical.switches.0.state", "/digin/state"));
}

void loop() {
  // We're storing the event loop in a static variable so that it's only
  // acquired once. Saves a few function calls per loop iteration.
  static auto event_loop = SensESPBaseApp::get_event_loop();
  event_loop->tick();
}
