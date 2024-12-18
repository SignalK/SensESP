#include <Arduino.h>

#include "sensesp/sensors/digital_output.h"
#include "sensesp/signalk/signalk_listener.h"
#include "sensesp/signalk/signalk_output.h"
#include "sensesp/signalk/signalk_value_listener.h"
#include "sensesp/transforms/threshold.h"
#include "sensesp_app.h"
#include "sensesp_app_builder.h"

using namespace sensesp;

// This example shows how to listen to a specific SK Path (on the SK Server) for
// a value within a specified range. If the value is in the range (or outside,
// if that's how you've set it up), the output will be "true". You can then
// connect to a DigitalOutput, to turn on an LED, or activate a relay to sound
// an alarm, or whatever. This example demonstrates how to turn on a navigation
// light when it gets dark outside.

void setup() {
  // Some initialization boilerplate when in debug mode...
  SetupLogging();

  // Create a builder object
  SensESPAppBuilder builder;

  // Create the global SensESPApp() object.
  sensesp_app = builder.set_hostname("relay")
                    ->set_sk_server("10.10.10.1", 3000)
                    ->set_wifi_client("yourSSID", "yourPassword")
                    ->get_app();

  // To find valid Signal K Paths that fits your need you look at this link:
  // https://signalk.org/specification/1.4.0/doc/vesselsBranch.html
  // Define the SK Path you want to listen to
  const char* sk_path = "environment.outside.illuminance";

  // The "Configuration path" is combined with "/config" to formulate a URL
  // used by the RESTful API for retrieving or setting configuration data.
  // It is ALSO used to specify a path to the SPIFFS file system
  // where configuration data is saved on the MCU board.  It should
  // ALWAYS start with a forward slash if specified.  If left blank,
  // that indicates this sensor or transform does not have any
  // configuration to save.
  // Note that if you want to be able to change the sk_path at runtime,
  // you will need to specify a config_path.
  const char* config_path = "/threshold/lights";

  // Wire up the output of the float value on server
  // "environment.outside.illuminance" to the NumericThreshold, and then output
  // the transformed float to boolean to DigitalOutput
  auto* listener = new FloatSKListener(sk_path);
  auto threshold = new FloatThreshold(0.0f, 100.0f, true, config_path);
  ConfigItem(threshold)->set_title("Threshold")->set_sort_order(1000);
  listener->connect_to(threshold)->connect_to(new DigitalOutput(5));
}

void loop() { event_loop()->tick(); }
