#include <Arduino.h>

#include "app.h"

ReactESP app([] () {
  Serial.begin(115200);

  // A small arbitrary delay is required to let the
  // serial port catch up
  delay(100);

  sensesp_app = new SensESPApp();
  sensesp_app->enable();
});
