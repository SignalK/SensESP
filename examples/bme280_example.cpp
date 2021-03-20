// BME280_example.cpp

#include <Arduino.h>

//#define SERIAL_DEBUG_DISABLED

#define USE_LIB_WEBSOCKET true

#include "sensesp_app.h"
#include "sensors/bme280.h"
#include "signalk/signalk_output.h"
#include "transforms/dew_point.h"
#include "transforms/air_density.h"

ReactESP app([]() {
#ifndef SERIAL_DEBUG_DISABLED
  SetupSerialDebug(115200);
#endif

  sensesp_app = new SensESPApp();

  // Create a BME280, which represents the physical sensor.
  // 0x77 is the default address. Some chips use 0x76, which is shown here.
  auto* bme280 = new BME280(0x76);

  // If you want to change any of the settings that are set by
  // Adafruit_BME280::setSampling(), do that here, like this:
  // bme280->adafruit_bme280->setSampling(); // pass in the parameters you want

  // Define the read_delays you're going to use:
  const uint read_delay = 1000;            // once per second
  const uint pressure_read_delay = 60000;  // once per minute

  // Create a BME280Value, which is used to read a specific value from the
  // BME280, and send its output to Signal K as a number (float). This one is for
  // the temperature reading.
  auto* bme_temperature =
      new BME280Value(bme280, BME280Value::temperature, read_delay, "/Outside/Temperature");

  bme_temperature->connect_to(
      new SKOutputNumber("environment.outside.temperature"));

  // Do the same for the barometric pressure value. Its read_delay is longer,
  // since barometric pressure can't change all that quickly. It could be much
  // longer for that reason.
  auto* bme_pressure = new BME280Value(bme280, BME280Value::pressure, pressure_read_delay,
                                       "/Outside/Pressure");

  bme_pressure->connect_to(new SKOutputNumber("environment.outside.pressure"));

  // Do the same for the humidity value.
  auto* bme_humidity =
      new BME280Value(bme280, BME280Value::humidity, read_delay, "/Outside/Humidity");

  bme_humidity->connect_to(new SKOutputNumber("environment.outside.humidity"));


  // Use the transform dewPoint to calculate the dewpoint based upon the temperature and humidity.
  auto* dew_point = new DewPoint();

  dew_point->connect_from(bme_temperature, bme_humidity)
          ->connect_to(new SKOutputNumber("environment.outside.dewPointTemperature"));

  // Use the transform airDensity to calculate the air density of humid air based
  // upon the temperature, humidity and pressure.
  auto* airDensity = new AirDensity();

  airDensity->connect_from(bme_temperature,bme_humidity,bme_pressure)
          ->connect_to(new SKOutputNumber("environment.inside.engineroom.airDensity"));


  sensesp_app->enable();
});
