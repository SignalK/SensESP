#include <Arduino.h>

#include "sensesp_app.h"
#include "signalk/signalk_output.h"
#include "signalk/signalk_listener.h"
#include "signalk/signalk_value_listener.h"
#include "sensors/digital_output.h"
#include "transforms/threshold.h"


// Following sample shows how to listen for value on Signal K server.
// This sample demonstrates how to make automatic navigation light switching device depending on light conditions.

// SensESP builds upon the ReactESP framework. Every ReactESP application
// defines an "app" object vs defining a "main()" method.
ReactESP app([] () {

  // Some initialization boilerplate when in debug mode...
  #ifndef SERIAL_DEBUG_DISABLED
  Serial.begin(115200);

  // A small arbitrary delay is required to let the
  // serial port catch up
  delay(100);
  Debug.setSerialEnabled(true);
  #endif


  // Create the global SensESPApp() object.
  sensesp_app = new SensESPApp();
  // The "SignalK path" identifies subscription of data from the SignalK network.
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
  auto * listener = new SKOutputNumber(sk_path);
  listener->connectTo(new NumericThreshold(0.0f, 100.0f, true, config_path), 0)
          ->connectTo(new DigitalOutput(5));
  // Start the SensESP application running
  sensesp_app->enable();
});