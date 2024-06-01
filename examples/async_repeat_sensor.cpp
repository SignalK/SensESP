/**
 * @file async_repeat_sensor.cpp
 * @brief Example of an asynchronous RepeatSensor that reads a digital input.
 *
 * RepeatSensor is a helper class that can be used to wrap any generic Arduino
 * sensor library into a SensESP sensor. This file demonstrates its use with
 * the built-in GPIO digital input, but in an asynchronous manner.
 *
 */

#include "sensesp/signalk/signalk_output.h"
#include "sensesp_app_builder.h"

using namespace sensesp;

reactesp::ReactESP app;

// The setup function performs one-time application initialization.
void setup() {
  SetupLogging();

  // Create the global SensESPApp() object.
  SensESPAppBuilder builder;
  sensesp_app = builder.get_app();

  // GPIO pin that we'll be using for the digital input.
  const uint8_t kDigitalInputPin = 15;

  // Do here any sensor initialization you need. For a more complex sensor,
  // this might include initializing the hardware, setting the I2C pins, etc.
  pinMode(kDigitalInputPin, INPUT_PULLUP);

  // define an asynchronous callback function that reads a digital input pin
  // after a delay of 1000 ms.
  auto digital_read_callback = [](RepeatSensor<bool>* sensor) {
    debugI("Pretend to trigger an asynchronous measurement operation here.");
    app.onDelay(1000,
                [sensor]() { sensor->emit(digitalRead(kDigitalInputPin)); });
  };

  // Let's read the sensor every 2000 ms.
  unsigned int read_interval = 2000;

  // Create a RepeatSensor with float output that reads the digital input.
  auto* digital = new RepeatSensor<bool>(read_interval, digital_read_callback);

  // The "Signal K path" identifies this sensor to the Signal K server.
  //
  // To find valid Signal K Paths that fits your need you look at this link:
  // https://signalk.org/specification/1.4.0/doc/vesselsBranch.html
  const char* sk_path = "environment.bool.pin15";

  // Connect the output of the digital input to the SKOutput object which
  // transmits the results to the Signal K server.
  digital->connect_to(new SKOutputFloat(sk_path, ""));
}

// The loop function is called in an endless loop during program execution.
// It simply calls `app.tick()` which will then execute all reactions as needed.
void loop() { app.tick(); }
