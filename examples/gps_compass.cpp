#include <Arduino.h>

// needed because the GPS module needs the only serial port
// on ESP8266
#define SERIAL_DEBUG_DISABLED

#include "sensesp_app.h"
#include "wiring_helpers.h"

ReactESP app([] () {
  sensesp_app = new SensESPApp();

  setup_gps(D7);

  sensesp_app->enable();
});
