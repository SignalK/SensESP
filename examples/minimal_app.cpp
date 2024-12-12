#include <math.h>

#include "sensesp/net/web/config_handler.h"
#include "sensesp/net/web/static_file_handler.h"
#include "sensesp/signalk/signalk_delta_queue.h"
#include "sensesp/signalk/signalk_ws_client.h"
#include "sensesp/system/button.h"
#include "sensesp_minimal_app_builder.h"

using namespace sensesp;

const unsigned int read_delay = 500;

// This is a sample program to demonstrate how to instantiate a
// SensESPMinimalApp application and only enable some required components
// explicitly.
//
// The program just wiggles GPIO pins 18 and 21 at defined rates, and
// reads the input of pins 15 and 13. If the output and input pins are connected
// with a jumper wire, you should see changes in the reported values on the
// serial console.

void setup() {
  SetupLogging();

  SensESPMinimalAppBuilder builder;
  auto sensesp_app = builder.set_hostname("counter-test")->get_app();

  auto button_handler = std::make_shared<ButtonHandler>(0);

  // As an example, we'll connect to WiFi manually using the Arduino ESP32 WiFi
  // library instead of using the SensESP Networking class.

  WiFi.mode(WIFI_STA);  // Optional
  WiFi.begin("ssid", "password");
  Serial.println("\nConnecting");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());

  // Initiate the objects you need for your application here. Have a look
  // at sensesp_app.h and pick the necessary items from there.

  // create the SK delta queue
  auto sk_delta_queue = std::make_shared<SKDeltaQueue>();

  // Use this if you want to hardcode the Signal K server address
  String sk_server_address = "openplotter.local";
  // Use this if you want to hardcode the Signal K server port
  uint16_t sk_server_port = 3000;
  // Set this to true if you want to use mDNS to discover the Signal K server
  bool use_mdns = false;

  auto ws_client =
      std::make_shared<SKWSClient>("/System/Signal K Settings", sk_delta_queue,
                                   sk_server_address, sk_server_port, use_mdns);

  // To avoid garbage collecting all shared pointers
  // created in setup(), loop from here.
  while (true) {
    loop();
  }
}

void loop() { event_loop()->tick(); }
