#include <Arduino.h>

#include "sensesp_app_builder.h"
#include "sensors/analog_input.h"
#include "signalk/signalk_output.h"
#include "transforms/moving_average.h"

#define SERIAL_DEBUG_DISABLED = true

using namespace sensesp;

ReactESP app;

void setup() {
#ifndef SERIAL_DEBUG_DISABLED
  SetupSerialDebug(115200);
#endif

  // Set up sensesp
  SensESPAppBuilder builder;
  auto sensesp_app = builder.get_app();

  // This reads A0 every 100 ms.
  // If you're using an ESP32, you must specify the pin number: AnalogInput(14);
  // You can also specify any read interval: AnalogInput(A0, 500);
  AnalogInput* input = new AnalogInput();

  // scale factor. this will depend on your circuit:
  // 1. Take the maximum analog input value (i.e. value when sensor is at the
  // high end)
  // 2. Divide 1 by max value. In my case: 1 / 870 = 0.001149425
  float scale = 0.001149425F;

  // Takes a moving average for every 10 values, with scale factor
  MovingAverage* avg = new MovingAverage(10, scale);

  // Connect avg. value to a SK path
  // See this link for available tank paths:
  // https://signalk.org/specification/1.4.0/doc/vesselsBranch.html#vesselsregexptanks
  input->connect_to(avg)->connect_to(
      new SKOutputFloat("tanks.fuel.0.currentLevel"));

  sensesp_app->start();
}

// The loop function is called in an endless loop during program execution.
// It simply calls `app.tick()` which will then execute all reactions as needed.
void loop() {
  app.tick();
}
