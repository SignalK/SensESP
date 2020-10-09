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
#ifdef ESP8266
  uint8_t pinA = D5;
  uint8_t pinB = D6;  
#elif defined(ESP32)
  uint8_t pinA = 4;
  uint8_t pinB = 5;
#endif
  setup_fuel_flow_meter(pinA, pinB);

  sensesp_app->enable();
});
