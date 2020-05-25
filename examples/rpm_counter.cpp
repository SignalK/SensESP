#include <Arduino.h>

//#define SERIAL_DEBUG_DISABLED

#include "sensors/digital_input.h"
#include "sensesp_app.h"
#include "transforms/frequency.h"
#include "signalk/signalk_output.h"


// SensESP builds upon the ReactESP framework. Every ReactESP application
// defines an "app" object vs defining a "main()" method.
ReactESP app([]() {
#ifndef SERIAL_DEBUG_DISABLED
  Serial.begin(115200);

  // A small delay and one debugI() are required so that
  // the serial output displays everything
  delay(100);
  Debug.setSerialEnabled(true);
  #endif
  delay(100);
  debugI("Serial debug enabled");

  sensesp_app = new SensESPApp();

  // The "SignalK path" identifies the output of the sensor to the SignalK network.
  // If you have multiple sensors connected to your microcontoller (ESP), each one of them
  // will (probably) have its own SignalK path variable. For example, if you have two
  // propulsion engines, and you want the RPM of each of them to go to SignalK, you might
  // have sk_path_portEngine = "propulsion.port.revolutions"
  // and sk_path_starboardEngine = "propulsion.starboard.revolutions"
  // In this example, there is only one propulsion engine, and its RPM is the only thing
  // being reported to SignalK.
  const char* sk_path = "propulsion.main.revolutions";


  // The "Configuration path" is combined with "/config" to formulate a URL
  // used by the RESTful API for retrieving or setting configuration data.
  // It is ALSO used to specify a path to the SPIFFS file system
  // where configuration data is saved on the microcontroller.  It should
  // ALWAYS start with a forward slash if specified. If left blank,
  // that indicates this sensor or transform does not have any
  // configuration to save.
  // Note that if you want to be able to change the sk_path at runtime,
  // you will need to specify a configuration path.
  // As with the SignalK path, if you have multiple configurable sensors
  // connected to the microcontroller, you will have a configuration path
  // for each of them, such as config_path_portEngine = "/sensors/portEngine/rpm"
  // and config_path_starboardEngine = "/sensor/starboardEngine/rpm".
  const char* config_path = "/sensors/engine_rpm";

  // These two are necessary until a method is created to synthesize them. Everything
  // after "/sensors" in each of these ("/engine_rpm/calibrate" and "/engine_rpm/sk")
  // is simply a label to display what you're configuring in the Configuration UI. 
  const char* config_path_calibrate = "/sensors/engine_rpm/calibrate";
  const char* config_path_skpath = "/sensors/engine_rpm/sk";


//////////
// connect a RPM meter. A DigitalInputCounter implements an interrupt
// to count pulses and reports the readings every read_delay ms
// (500 in the example). A Frequency
// transform takes a number of pulses and converts that into
// a frequency. The sample multiplier converts the 97 tooth
// tach output into Hz, SK native units.
const float multiplier = 1.0 / 97.0;
const uint read_delay = 500;


  // Wire it all up by connecting the producer directly to the consumer
  // ESP8266 pins are specified as DX
  // ESP32 pins are specified as just the X in GPIOX
  auto* pSensor = new DigitalInputCounter(D5, INPUT_PULLUP, RISING, read_delay);

  pSensor->connectTo(new Frequency(multiplier, config_path_calibrate))  // connect the output of pSensor to the input of Frequency()
        ->connectTo(new SKOutputNumber(sk_path, config_path_skpath));   // connect the output of Frequency() to a SignalK Output as a number


  // Start the SensESP application running. Because of everything that's been set up above,
  // it constantly monitors the interrupt pin, and every read_delay ms, it sends the 
  // calculated frequency to SignalK.
  sensesp_app->enable();
});

