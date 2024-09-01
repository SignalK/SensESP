
#include "sensesp/sensors/analog_input.h"

#include <Arduino.h>

#include "sensesp/signalk/signalk_output.h"
#include "sensesp/transforms/linear.h"
#include "sensesp_app.h"
#include "sensesp_app_builder.h"

using namespace sensesp;

// SensESP builds upon the ReactESP framework. Every ReactESP application
// must instantiate the "app" object.
reactesp::EventLoop app;

// The setup function performs one-time application initialization.
void setup() {
  // Some initialization boilerplate when in debug mode...
  SetupLogging();

  // Create the global SensESPApp() object.
  SensESPAppBuilder builder;
  sensesp_app = builder.get_app();

  // The "Signal K path" identifies this sensor to the Signal K server. Leaving
  // this blank would indicate this particular sensor (or transform) does not
  // broadcast Signal K data.
  // To find valid Signal K Paths that fits your need you look at this link:
  // https://signalk.org/specification/1.4.0/doc/vesselsBranch.html
  const char* sk_path = "environment.indoor.illuminance";

  // The "Configuration path" is combined with "/config" to formulate a URL
  // used by the RESTful API for retrieving or setting configuration data.
  // It is ALSO used to specify a path to the file system
  // where configuration data is saved on the MCU board. It should
  // ALWAYS start with a forward slash if specified. If left blank,
  // that indicates this sensor or transform does not have any
  // configuration to save, or that you're not interested in doing
  // run-time configuration.
  const char* analog_in_config_path = "/indoor_illuminance/analog_in";
  const char* linear_config_path = "/indoor_illuminance/linear";

  // Create a sensor that is the source of the data, which will be read every
  // 500 ms. It's a light sensor that's connected to the ESP's AnalogIn pin.
  // ESP32 has many pins that can be
  // used for AnalogIn, and they're expressed here as the XX in GPIOXX.
  // When it's dark, the sensor's output (as read by analogRead()) is 120, and
  // when it's bright, the output is 850, for a range of 730.
  uint8_t pin = 34;
  unsigned int read_delay = 500;

  auto* analog_input = new AnalogInput(pin, read_delay, analog_in_config_path);

  // A Linear transform takes its input, multiplies it by the multiplier, then
  // adds the offset, to calculate its output. In this example, the final output
  // will be presented as a ratio, where dark = 0.0 and bright = 1.0.
  // (In the final output, on a gauge in Instrument Panel, for example, it will
  // look like a percentage from 0 to 100, but the official "unit"  in the
  // Signal K specification for a percentage is "ratio".)
  // See
  // https://github.com/SignalK/specification/blob/master/schemas/definitions.json#L87
  // for more details.
  //
  // First, calculate the multiplier. Since the total measurement range
  // is 730 "analogRead units", and that will be represented by a total
  // output range of 1.000 (from 0.000 to 1.000), divide 1.000 by 730 to
  // get the "value per analogRead unit": 1.000 / 730 = 0.00137
  //
  // To calculate the offset, multiply the analogRead units at Dark times
  // the multiplier: 120 * 0.00137 = 0.1644
  // and make it negative, because the final value for "dark" needs to be 0.000.
  //
  // Proof:
  // Dark = 120 analogRead units x 0.00137 = 0.1644
  //        0.1644 + -0.1644 = 0.000
  // Bright = 850 analogRead units x 0.00137 = 1.1645
  //        1.1645 + -0.1644 = 1.0001 (rounds to 1.000)

  const float multiplier = 0.00137;
  const float offset = -0.1644;

  // Connect the output of the analog input to the Linear transform,
  // and then output the results to the Signal K server. As part of
  // that output, send some metadata to indicate that the "units"
  // to be used to display this value is "ratio". Also specify that
  // the display name for this value, to be used by any Signal K
  // consumer that displays it, is "Indoor light".
  analog_input->connect_to(new Linear(multiplier, offset, linear_config_path))
      ->connect_to(new SKOutputFloat(sk_path, "",
                                     new SKMetadata("ratio", "Indoor light")));
}

// The loop function is called in an endless loop during program execution.
// It simply calls `app.tick()` which will then execute all events as needed.
void loop() { app.tick(); }
