/* 
 * This is the most basic example we could come up with to demonstrate
 * a SensESP program that sends data to a Signal K Server. Comments
 * here in the program file are minimal - for a full explanation,
 * see https://github.com/SignalK/SensESP/pages/tutorials/minimal_sk
 * 
 * Once the program is running, simply connect / disconnect a jumper
 * wire between GND and GPIO7 to toggle the value of the
 * digital.pin7.state Signal K Path between TRUE and FALSE.
*/

#include <Arduino.h>
#include "sensesp_app.h"
#include "sensesp_app_builder.h"
#include "signalk/signalk_output.h"
#include "sensors/digital_input.h"

using namespace sensesp;

// SensESP builds upon the ReactESP framework. Every ReactESP application
// must instantiate the "app" object.
reactesp::ReactESP app;

// The setup function performs one-time application initialization
void setup() {

// Some initialization boilerplate when in debug mode
#ifndef SERIAL_DEBUG_DISABLED
  SetupSerialDebug(115200);
#endif

  // Create the global SensESPApp() object
  SensESPAppBuilder builder;
  sensesp_app = builder.get_app();

  // Set the Signal K Path for the output
  const char* sk_path = "digitalPin.7.state";

  // Define the GPIO pin to read
  uint8_t pin = 7;

  // Define how often to read the state of the pin, in ms
  uint16_t read_delay = 500;

  auto* pin_state = new DigitalInputState(pin, INPUT_PULLUP, read_delay);

  // Send the value of the digital pin to the Signal K server as a Boolean
  pin_state->connect_to(new SKOutputBool(sk_path));

  // Start the SensESP application running
  sensesp_app->start();

} //setup

// loop simply calls `app.tick()` which will then execute all reactions as needed
void loop() {
  app.tick();
}
