#include <Arduino.h>

#include "sensesp_app.h"
#include "devices/analog_input.h"
#include "transforms/linear.h"

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


  // The "SignalK path" identifies this sensor to the SignalK network. Leaving
  // this blank would indicate this particular sensor (or transform) does not
  // broadcast SignalK data
  const char* sk_path = "sensors.indoor.illumination";


  // The "Configuration path" is combined with "/config" to formulate a URL
  // used by the RESTful API for retrieving or setting configuration data.
  // It is ALSO used to specify a path to the SPIFFS file system
  // where configuration data is saved on the MCU board.  It should 
  // ALWAYS start with a forward slash if specified.  If left blank, 
  // that indicates this sensor or transform does not have any 
  // configuration to save.
  // Note that if you want to be able to change the sk_path at runtime,
  // you will need to specify a config_path.
  const char* config_path = "/sensors/indoor_illumination";


  // Create a "device" that is the source of our data
  auto* pAnalogInput = new AnalogInput();


  // Create a "transform" that can modify the data and/or broadcast it
  // over the SignalK network.
  const float multiplier = 1.0;
  const float offset = 0.0;
  auto* pTransform = new Linear(sk_path, multiplier, offset, config_path);

  
  // Wire up the output of the analog input to the transform
  pAnalogInput->connectTo(pTransform);


  // Start the SensESP application running
  sensesp_app->enable();
});
