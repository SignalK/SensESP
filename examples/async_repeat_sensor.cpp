/**
 * @file async_repeat_sensor.cpp
 * @brief Example of an asynchronous RepeatSensor that reads a digital input.
 *
 * RepeatSensor is a helper class that can be used to wrap any generic Arduino
 * sensor library into a SensESP sensor. This file demonstrates its use with
 * the built-in GPIO digital input, but in an asynchronous manner.
 *
 */

#include "sensesp_app_builder.h"
#include "signalk/signalk_output.h"
#include "transforms/linear.h"

using namespace sensesp;

reactesp::ReactESP app;

// The setup function performs one-time application initialization.
void setup() {
// Some initialization boilerplate when in debug mode...
#ifndef SERIAL_DEBUG_DISABLED
  SetupSerialDebug(115200);
#endif

  // Create the global SensESPApp() object.
  SensESPAppBuilder builder;
  sensesp_app = builder.get_app();

  const uint8_t kDigitalInputPin = 15;

  // Do here any sensor initialization you need. For a more complex sensor,
  // this might include initializing the hardware, setting the I2C pins, etc.
  pinMode(kDigitalInputPin, INPUT_PULLUP);

  // GPIO pin that we'll be using for the analog input.

  // define an asynchronous callback function that reads a digital input pin
  // after a delay of 1000 ms.
  auto analog_read_callback = [](RepeatSensor<bool>* sensor) {
    debugI("Pretend to trigger an asynchronous measurement operation here.");
    app.onDelay(
        1000, [sensor]() { sensor->emit(digitalRead(kDigitalInputPin)); });
  };

  // Let's read the sensor every 2000 ms.
  unsigned int read_interval = 2000;

  // Create a RepeatSensor with float output that reads the analog input.
  auto* digital =
      new RepeatSensor<bool>(read_interval, analog_read_callback);

  // The "Signal K path" identifies this sensor to the Signal K server. Leaving
  // this blank would indicate this particular sensor (or transform) does not
  // broadcast Signal K data.
  // To find valid Signal K Paths that fits your need you look at this link:
  // https://signalk.org/specification/1.4.0/doc/vesselsBranch.html
  const char* sk_path = "environment.bool.pin15";

  // Connect the output of the analog input to the Linear transform,
  // and then output the results to the Signal K server. As part of
  // that output, send some metadata to indicate that the "units"
  // to be used to display this value is "ratio". Also specify that
  // the display name for this value, to be used by any Signal K
  // consumer that displays it, is "Indoor light".
  digital->connect_to(
      new SKOutputFloat(sk_path, ""));

  // Start the SensESP application running
  sensesp_app->start();
}

// The loop function is called in an endless loop during program execution.
// It simply calls `app.tick()` which will then execute all reactions as needed.
void loop() { app.tick(); }
