#include "app.h"

#include <Arduino.h>

SensESPApp* sensesp_app;

ReactESP app([] () {
  Serial.begin(115200);

  sensesp_app = new SensESPApp();
  sensesp_app->enable();
});
