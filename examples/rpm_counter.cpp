#include <Arduino.h>

//#define SERIAL_DEBUG_DISABLED

#include "devices/digital_input.h"
#include "sensesp_app.h"
#include "transforms/frequency.h"
#include "wiring_helpers.h"


// SensESP builds upon the ReactESP framework. Every ReactESP application
// defines an "app" object vs defining a "main()" method.
ReactESP app([]() {
#ifndef SERIAL_DEBUG_DISABLED
  Serial.begin(115200);

  // A small arbitrary delay is required to let the
  // serial port catch up

  delay(100);
  Debug.setSerialEnabled(true);
#endif

  sensesp_app = new SensESPApp();



  // The "SignalK path" identifies this sensor to the SignalK network. Leaving
  // this blank would indicate this particular sensor (or transform) does not
  // broadcast SignalK data
  const char* sk_path = "propulsion.left.revolutions";


  // The "Configuration path" is combined with "/config" to formulate a URL
  // used by the RESTful API for retrieving or setting configuration data.
  // It is ALSO used to specify a path to the SPIFFS file system
  // where configuration data is saved on the MCU board.  It should 
  // ALWAYS start with a forward slash if specified.  If left blank, 
  // that indicates this sensor or transform does not have any 
  // configuration to save.
  // Note that if you want to be able to change the sk_path at runtime,
  // you will need to specify a config_path.
  const char* config_path = "/sensors/engine_rpm";



//////////
// connect a RPM meter. A DigitalInputCounter counts pulses
// and reports the readings every read_delay ms
// (500 in the example). A Frequency
// transform takes a number of pulses and converts that into
// a frequency. The sample multiplier converts the 97 tooth
// tach output into Hz, SK native units.
const float multiplier = 1.0 / 97.0;
const uint read_delay = 500;


  // Two ways to wire up devices and transformations:

  // 1. Connect by specifying data type of Producer's output/Consumer's input
  /*
     sensesp_app->connect<int>(
          new DigitalInputCounter(D5, INPUT_PULLUP, RISING, read_delay),
          new Frequency(sk_path, multiplier, config_path)
     );
  */

  // 2. Connect the producer directly to the consumer
  auto* pDevice = new DigitalInputCounter(D5, INPUT_PULLUP, RISING, read_delay);
  
  pDevice->connectTo(new Frequency(sk_path, multiplier, config_path));


  // Start the SensESP application running
  sensesp_app->enable();
});
