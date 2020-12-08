#include "sensors/analog_input.h"

#include <Arduino.h>

#include "sensesp_app.h"
#include "signalk/signalk_output.h"
#include "transforms/linear.h"

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
  const char* sk_path = "environment.indoor.illuminance";

  // The "Configuration path" is combined with "/config" to formulate a URL
  // used by the RESTful API for retrieving or setting configuration data.
  // It is ALSO used to specify a path to the SPIFFS file system
  // where configuration data is saved on the MCU board. It should
  // ALWAYS start with a forward slash if specified. If left blank,
  // that indicates this sensor or transform does not have any
  // configuration to save, or that you're not interested in doing
  // run-time configuration.
  const char* analog_in_config_path = "/indoor_illuminance/analog_in";
  const char* linear_config_path = "/indoor_illuminance/linear";

  // Create a sensor that is the source of our data, that will be read every 500
  // ms. It's a light sensor that's connected to the ESP's AnalogIn pin. The
  // AnalogIn pin on ESP8266 is always A0, but ESP32 has many pins that can be
  // used for AnalogIn, and they're expressed here as the XX in GPIOXX.
  // When it's dark, the sensor's output (as read by analogRead()) is 120, and
  // when it's bright, the output is 850, for a range of 730.
  uint8_t pin = A0;
  uint read_delay = 500;

  auto* analog_input = new AnalogInput(pin, read_delay, analog_in_config_path);

  // A Linear transform takes its input, multiplies it by the multiplier, then
  // adds the offset, to calculate its output. In this example, we want to see
  // the final output presented as a percentage, where dark = 0% and bright =
  // 100%. To get a percentage, we use this formula: sensor output * (100 / 730)
  // - 16.44 = output (a percentage from 0 to 100). Dark = 120 * (100 / 730) +
  // (-16.44) = 0% Bright = 850 * (100 / 730) + (-16.44) = 100%
  const float multiplier =
      0.137;  // 100% divided by 730 = 0.137 "percent per analogRead() unit"
  const float offset = -16.44;

  // Wire up the output of the analog input to the Linear transform,
  // and then output the results to the Signal K server. As part of
  // that output, send some metadata to indicate that the "units"
  // of the value we are sending is a "ratio" - which is the official
  // unit type for a percentage represented as a float between 0.0 and 1.0
  // See https://github.com/SignalK/specification/blob/de83f65f0144bacde665be547d08eb9a12c70212/schemas/definitions.json#L82
  // for more details.
  analog_input->connect_to(new Linear(multiplier, offset, linear_config_path))
      ->connect_to(new SKOutputNumber(sk_path, "", new SKMetadata("ratio")));

  // Start the SensESP application running
  sensesp_app->enable();
});
