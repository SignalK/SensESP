#include <Arduino.h>

#include "sensesp_app.h"
#include "wiring_helpers.h"

ReactESP app([]() {
#ifndef SERIAL_DEBUG_DISABLED
  SetupSerialDebug(115200);
#endif

  sensesp_app = new SensESPApp();

  // setup the fuel flow meter on two pins
  // ESP8266 pins are specified as DX
  // ESP32 pins are specified as just the X in GPIOX
  setup_fuel_flow_meter(D5, D6);

  sensesp_app->enable();
});
