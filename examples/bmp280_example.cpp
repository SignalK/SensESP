// BMP280_example.cpp

#include <Arduino.h>

//#define SERIAL_DEBUG_DISABLED

#define USE_LIB_WEBSOCKET true

#include "sensesp_app.h"
#include "signalk/signalk_output.h"
#include "sensors/bmp280.h"

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

  // Create the SensESPApp with whatever "standard sensors" you want: noStdSensors, allStdSensors, or uptimeOnly.
  // The default is allStdSensors.
  sensesp_app = new SensESPApp(uptimeOnly);

  // Create a BMP280, which represents the physical sensor.
  // 0x77 is the default address. Some chips use 0x76, which is shown here.
  auto* pBMP280 = new BMP280(0x76);

  // If you want to change any of the settings that are set by Adafruit_BMP280::setSampling(), do
  // that here, like this:
  // pBMP280->pAdafruitBMP280->setSampling(); // pass in the parameters you want

  // Define the read_delays you're going to use:
  const uint read_delay = 1000; // once per second
  const uint pressure_read_delay = 60000; // once per minute

  // Create a BMP280value, which is used to read a specific value from the BMP280, and send its output
  // to SignalK as a number (float). This one is for the temperature reading.
  auto* pBMPtemperature = new BMP280value(pBMP280, temperature, read_delay, "/Outside/Temperature");
      
      pBMPtemperature->connectTo(new SKOutputNumber("environment.outside.temperature"));


  // Do the same for the barometric pressure value. Its read_delay is longer, since barometric pressure can't
  // change all that quickly. It could be much longer for that reason.
  auto* pBMPpressure = new BMP280value(pBMP280, pressure,  pressure_read_delay, "/Outside/Pressure");
      
      pBMPpressure->connectTo(new SKOutputNumber("environment.outside.pressure"));


    sensesp_app->enable();
});
