#include <Arduino.h>

#include "sensesp_app.h"
#include "wiring_helpers.h"

ReactESP app([] () {
  #ifndef SERIAL_DEBUG_DISABLED
  Serial.begin(115200);

  // A small delay and one debugI() are required so that
  // the serial output displays everything
  delay(100);
  Debug.setSerialEnabled(true);
  #endif
  delay(100);
  debugI("Serial debug enabled");

  sensesp_app = new SensESPApp();

  // setup the fuel flow meter on two pins
  // ESP8266 pins are specified as DX
  // ESP32 pins are specified as just the X in GPIOX
  setup_fuel_flow_meter(D5, D6);

  sensesp_app->enable();
});
