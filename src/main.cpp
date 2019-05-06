#include <Arduino.h>

//#define SERIAL_DEBUG_DISABLED

#include "app.h"
#include "wiring_helpers.h"

ReactESP app([] () {
  #ifndef SERIAL_DEBUG_DISABLED
  Serial.begin(115200);

  // A small arbitrary delay is required to let the
  // serial port catch up

  delay(100);
  Debug.setSerialEnabled(true);
  #endif

  sensesp_app = new SensESPApp();

  setup_analog_input(
    sensesp_app,
    "sensors.indoor.illumination",
    1, 0,
    "/sensors/indoor_illumination");

  //setup_gps(sensesp_app, 5);

  sensesp_app->enable();
});
