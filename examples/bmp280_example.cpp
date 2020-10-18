// BMP280_example.cpp

#include <Arduino.h>

//#define SERIAL_DEBUG_DISABLED

#define USE_LIB_WEBSOCKET true

#include "sensesp_app.h"
#include "sensors/bmp280.h"
#include "signalk/signalk_output.h"

ReactESP app([]() {
#ifndef SERIAL_DEBUG_DISABLED
  SetupSerialDebug(115200);
#endif

  sensesp_app = new SensESPApp();

  // Create a BMP280, which represents the physical sensor.
  // 0x77 is the default address. Some chips use 0x76, which is shown here.
  auto* bmp280 = new BMP280(0x76);

  // If you want to change any of the settings that are set by
  // Adafruit_BMP280::setSampling(), do that here, like this:
  // bmp280->adafruit_bmp280->setSampling(); // pass in the parameters you want

  // Define the read_delays you're going to use:
  const uint read_delay = 1000;            // once per second
  const uint pressure_read_delay = 60000;  // once per minute

  // Create a BMP280Value, which is used to read a specific value from the
  // BMP280, and send its output to Signal K as a number (float). This one is for
  // the temperature reading.
  auto* bmp_temperature =
      new BMP280Value(bmp280, temperature, read_delay, "/Outside/Temperature");

  bmp_temperature->connect_to(
      new SKOutputNumber("environment.outside.temperature"));

  // Do the same for the barometric pressure value. Its read_delay is longer,
  // since barometric pressure can't change all that quickly. It could be much
  // longer for that reason.
  auto* bmp_pressure = new BMP280Value(bmp280, pressure, pressure_read_delay,
                                       "/Outside/Pressure");

  bmp_pressure->connect_to(new SKOutputNumber("environment.outside.pressure"));

  sensesp_app->enable();
});
