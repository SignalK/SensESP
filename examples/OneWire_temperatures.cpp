#include <Arduino.h>

//#define SERIAL_DEBUG_DISABLED

#define USE_LIB_WEBSOCKET true

#include "sensesp_app.h"
#include "sensors/onewire_temperature.h"
#include "signalk/signalk_output.h"
#include "wiring_helpers.h"
#include "transforms/linear.h"

ReactESP app([] () {
  #ifndef SERIAL_DEBUG_DISABLED
  Serial.begin(115200);

  // A small arbitrary delay is required to let the
  // serial port catch up

  delay(100);
  Debug.setSerialEnabled(true);
  #endif

  sensesp_app = new SensESPApp();

  /* Find all the sensors and their unique addresses. Then, each new instance
     of OneWireTemperature will use one of those addresses. You can't specify
     which address will initially be assigned to a particular sensor, so if you
     have more than one sensor, you may have to swap the addresses around on
     the configuration page for the device. (You get to the configuration page
     by entering the IP address of the device into a browser.)
  */
  DallasTemperatureSensors* dts = new DallasTemperatureSensors(D7);

  auto* pCoolantTemp = new OneWireTemperature(dts, "/coolantTemperature/oneWire");

    pCoolantTemp->connectTo(new Linear(1.0, 0.0, "/coolantTemperature/linear"))
                ->connectTo(new SKOutputNumber("propulsion.mainEngine.coolantTemperature", "/coolantTemperature/skPath"));

  auto* pExhaustTemp = new OneWireTemperature(dts, "/exhaustTemperature/oneWire");
    
    pExhaustTemp->connectTo(new Linear(1.0, 0.0, "/exhaustTemperature/linear"))
                ->connectTo(new SKOutputNumber("propulsion.mainEngine.exhaustTemperature", "/exhaustTemperature/skPath"));
  
  auto* p24VTemp = new OneWireTemperature(dts, "/24vAltTemperature/oneWire");
      
      p24VTemp->connectTo(new Linear(1.0, 0.0, "/24vAltTemperature/linear"))
              ->connectTo(new SKOutputNumber("electrical.alternators.24V.temperature", "/24vAltTemperature/skPath"));

  auto* p12VTemp = new OneWireTemperature(dts, "/12vAltTemperature/oneWire");
      
      p12VTemp->connectTo(new Linear(1.0, 0.0, "/12vAltTemperature/linear"))
              ->connectTo(new SKOutputNumber("electrical.alternators.12V.temperature", "/12vAltTemperature/skPath"));      

  sensesp_app->enable();
});
