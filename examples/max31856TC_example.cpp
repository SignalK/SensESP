#include <Arduino.h>

#include "sensesp_app.h"
#include "transforms/linear.h"
#include "signalk/signalk_output.h"
#include "sensors/max31856TC_input.h"

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


  // The "SignalK path" identifies this sensor to the SignalK server. Leaving
  // this blank would indicate this particular sensor (or transform) does not
  // broadcast SignalK data.
  const char* sk_path = "propulsion.Main_Engine.temperature";


  // The "Configuration path" is combined with "/config" to formulate a URL
  // used by the RESTful API for retrieving or setting configuration data.
  // It is ALSO used to specify a path to the SPIFFS file system
  // where configuration data is saved on the MCU board. It should
  // ALWAYS start with a forward slash if specified. If left blank,
  // that indicates this sensor or transform does not have any
  // configuration to save, or that you're not interested in doing
  // run-time configuration.
  const char* temperature_in_config_path = "/propulsion/Main_Engine/temperature_in";
  const char* linear_config_path = "/Main_Engine/temperature_in/linear";
  

  // Create a sensor that is the source of our data, that will be read every 500 ms. 
  // It is an ultrasonic distance sensor that sends out an acoustical pulse in response
  // to a 100 micro-sec trigger pulse from the ESP. The return acoustical pulse width
  // can be converted to a distance by the formula 2*distance = pulse_width/speed_of_sound
  // With pulse_width om ,icro-sec and distance in cm, 2*speed_of_sound = 58
  // The sensor is mounted at the top of a water tank that is 25 cm deep.
  uint read_delay = 500;
  
  auto* pTemperatureInput = new MAX31856TCInput(read_delay, temperature_in_config_path);

  // A Linear transform takes its input, multiplies it by the multiplier, then adds the offset,
  // to calculate its output. The MAX31856TC produces temperatures in degrees Celcius. We need to change
  // them to Kelvin for compatibility with SignalK.
 
  const float multiplier = 1.0;
  const float offset = 273.16;

  // Wire up the output of the analog input to the Linear transform,
  // and then output the results to the SignalK server.
  pTemperatureInput -> connectTo(new Linear(multiplier, offset, linear_config_path))
               -> connectTo(new SKOutputNumber(sk_path));

  // Start the SensESP application running
  sensesp_app->enable();
});