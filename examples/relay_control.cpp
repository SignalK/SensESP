#include <Arduino.h>

#include "sensesp_app.h"
#include "signalk/signalk_output.h"
#include "signalk/signalk_listener.h"
#include "signalk/signalk_value_listener.h"
#include "sensors/digital_output.h"
#include "transforms/threshold.h"


// This example shows how to listen to a specific SK Path (on the SK Server) for a value within a specified range.
// If the value is in the range (or outside, if that's how you've set it up), the output will be "true".
// You can then connect to a DigitalOutput, to turn on an LED, or activate a relay to sound an alarm, or whatever.
// This example demonstrates how to turn on a navigation light when it gets dark outside.

// SensESP builds upon the ReactESP framework. Every ReactESP application
// defines an "app" object (vs. defining a "main()" method).
ReactESP app([] () {

  // Some initialization boilerplate when in debug mode...
  #ifndef SERIAL_DEBUG_DISABLED
  Serial.begin(115200);

  // A small delay and one debugI() are required so that
  // the serial output displays everything
  delay(100);
  Debug.setSerialEnabled(true);
  #endif
  delay(100);
  debugI("Serial debug enabled");


  // Create the global SensESPApp() object.
  sensesp_app = new SensESPApp();
  
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
  
  // Wire up the output of the float value on server "environment.outside.illuminance" to the NumericThreshold,
  // and then output the transformed float to boolean to DigitalOutput
  auto * pListener = new SKNumericListener(sk_path);
  pListener->connectTo(new NumericThreshold(0.0f, 100.0f, true, config_path))
           ->connectTo(new DigitalOutput(5));
  
  // Start the SensESP application running
  sensesp_app->enable();
});