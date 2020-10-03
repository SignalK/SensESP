// SHT31_example.cpp

#include <Arduino.h>

//#define SERIAL_DEBUG_DISABLED

#define USE_LIB_WEBSOCKET true

#include "sensesp_app.h"
#include "sensors/sht31.h"
#include "signalk/signalk_output.h"

ReactESP app([]() {
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

  // Create a SHT31, which represents the physical sensor.
  // 0x44 is the default address. Some chips use 0x45, which is shown here.
  auto* sht31 = new SHT31(0x45);

  // Define the read_delay you're going to use. The default is 500 ms.
  const uint read_delay = 1000;

  // Create a SHT31value, which is used to read a specific value from the SHT31,
  // and send its output to SignalK as a Number (float). This one is for the
  // temperature reading.
  auto* sht31_temperature =
      new SHT31value(sht31, temperature, read_delay, "/fridge/temperature");

  sht31_temperature->connect_to(
      new SKOutputNumber("environment.inside.refrigerator.temperature"));

  // Do the same for the humidity value.
  auto* sht31_humidity =
      new SHT31value(sht31, humidity, read_delay, "/fridge/humidity");

  sht31_humidity->connect_to(
      new SKOutputNumber("environment.inside.refrigerator.humidity"));

  sensesp_app->enable();
});
