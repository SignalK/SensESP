#include <Adafruit_MAX31856.h>
#include <Arduino.h>

#include "sensesp_app.h"
#include "sensors/max31856_thermocouple.h"
#include "signalk/signalk_output.h"
#include "transforms/linear.h"

#define SPI_CS_PIN 15
#define SPI_MOSI_PIN 13
#define SPI_MISO_PIN 12
#define SPI_CLK_PIN 14
// There is a version of the constructor for the MAX31856Thermocouple that can use
// "hardware SPI", so you don't have to specify the pins.
// See https://github.com/SignalK/SensESP/blob/master/src/sensors/max31856_thermocouple.h
// and https://learn.adafruit.com/adafruit-max31856-thermocouple-amplifier/wiring-and-test

// SensESP builds upon the ReactESP framework. Every ReactESP application
// defines an "app" object vs defining a "main()" method.
ReactESP app([]() {

// Some initialization boilerplate when in debug mode...
#ifndef SERIAL_DEBUG_DISABLED
  SetupSerialDebug(115200);
#endif

  // Create the global SensESPApp() object.
  sensesp_app = new SensESPApp();

  // The "Signal K path" identifies this sensor to the Signal K server. Leaving
  // this blank would indicate this particular sensor (or transform) does not
  // broadcast Signal K data.
  // To find valid Signal K Paths that fits your need you look at this link:
  // https://signalk.org/specification/1.4.0/doc/vesselsBranch.html
  const char* sk_path = "propulsion.Main_Engine.exhaustTemperature";

  // The "Configuration path" is combined with "/config" to formulate a URL
  // used by the RESTful API for retrieving or setting configuration data.
  // It is ALSO used to specify a path to the SPIFFS file system
  // where configuration data is saved on the MCU board. It should
  // ALWAYS start with a forward slash if specified. If left blank,
  // that indicates this sensor or transform does not have any
  // configuration to save, or that you're not interested in doing
  // run-time configuration.

  const char* exhaust_temp_config_path =
      "/propulsion/Main_Engine/exhaustTemperature/read_delay";
  // const char* linear_config_path =
  // "/propulsion/Main_Engine/exhaustTemperature/linear";

  // Create a sensor that is the source of our data, that will be read every
  // 1000 ms.
  const uint readDelay = 1000;
  
  // tcType:  MAX31856_TCTYPE_K;   // other types can be B, E, J, N, R, S, T
  // There is an alternate constructor that allows you to create the
  // Adafruit_MAX31856 object however you like.
  // See https://github.com/SignalK/SensESP/blob/master/src/sensors/max31856_thermocouple.h
  auto* max31856tc = new MAX31856Thermocouple(
      SPI_CS_PIN, SPI_MOSI_PIN, SPI_MISO_PIN, SPI_CLK_PIN,
      MAX31856_TCTYPE_K, readDelay, exhaust_temp_config_path);

  // A Linear transform takes its input, multiplies it by the multiplier, then
  // adds the offset, to calculate its output. The MAX31856TC produces
  // temperatures in degrees Celsius. We need to change them to Kelvin for
  // compatibility with Signal K.

  const float multiplier = 1.0;
  const float offset = 273.16;

  // Wire up the output of the analog input to the Linear transform,
  // and then output the results to the Signal K server.
  max31856tc->connect_to(new Linear(multiplier, offset, ""))
      ->connect_to(new SKOutputNumber(sk_path));

  // Start the SensESP application running
  sensesp_app->enable();
});