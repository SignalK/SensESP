#include <Arduino.h>
#include "sensesp_app.h"
#include "system/valueconsumer.h"
#include "devices/analog_input.h"
#include "transforms/moving_average.h"
#include "wiring_helpers.h"

#define SERIAL_DEBUG_DISABLED = true

ReactESP app([] () {
  #ifndef SERIAL_DEBUG_DISABLED
    Serial.begin(115200);
    // A small arbitrary delay is required to let the
    // serial port catch up
    delay(100);
    Debug.setSerialEnabled(true);
  #endif

  // Set up sensesp
  sensesp_app = new SensESPApp();

  // This reads A0 every 100 ms
  AnalogInput* input = new AnalogInput();

  // scale factor. this will depend on your circuit:
  // 1. Take the maximum analog input value (i.e. value when sensor is at the high end)
  // 2. Divide 1 by max value. In my case: 1 / 870 = 0.001149425
  float scale = 0.001149425F;

  // Takes a moving average for every 10 values, with scale factor
  MovingAverage* avg = new MovingAverage("tanks.fuel.0.currentLevel", 10, scale);

  input->connectTo(avg);
  sensesp_app->enable();
});
