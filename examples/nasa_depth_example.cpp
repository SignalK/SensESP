#include <Arduino.h>

//#define SERIAL_DEBUG_DISABLED

#define USE_LIB_WEBSOCKET true

#include <sensors/digital_input.h>
#include "sensesp_app.h"
#include "signalk/signalk_output.h"
#include "wiring_helpers.h"
#include "sensors/nasa_depth.h"

ReactESP app([] () {
  #ifndef SERIAL_DEBUG_DISABLED
  Serial.begin(115200);

  // A small arbitrary delay is required to let the
  // serial port catch up

  delay(100);
  Debug.setSerialEnabled(true);
  #endif

  sensesp_app = new SensESPApp();

  const char* sk_path = "environment.depth.belowTransducer";
  const char* config_path_skpath = "/depth/sk";
  const uint read_delay = 500;

  auto* depth = new NasaDepth(D2, D1, read_delay, "/depth");
      
  depth->connectTo(new SKOutputNumber(sk_path, config_path_skpath));      

  sensesp_app->enable();
});
