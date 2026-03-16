
#include "sensesp/sensors/sensor.h"

#include <Arduino.h>

#include "sensesp/signalk/signalk_output.h"
#include "sensesp/transforms/linear.h"
#include "sensesp_app.h"
#include "sensesp_app_builder.h"

using namespace sensesp;

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
  const char* linear_config_path = "/indoor_illuminance/linear";

  // Create a sensor that is the source of the data, which will be read every
  // 500 ms. It's a light sensor that's connected to the ESP's analog input pin.
  // ESP32 has many pins that can be
  // used for analog input, and they're expressed here as the XX in GPIOXX.
  // When it's dark, the sensor's output voltage is about 0.097V, and
  // when it's bright, the output is about 0.685V, for a range of 0.588V.
  const uint8_t pin = 34;
  unsigned int read_delay = 500;

  auto* analog_input = new RepeatSensor<float>(read_delay, [pin]() {
    return analogReadMilliVolts(pin) / 1000.;
  });

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
  // is 0.588V, and that will be represented by a total output range of
  // 1.000 (from 0.000 to 1.000), divide 1.000 by 0.588 to get the
  // "value per volt": 1.000 / 0.588 = 1.7007
  //
  // To calculate the offset, multiply the voltage at Dark times
  // the multiplier: 0.097 * 1.7007 = 0.1650
  // and make it negative, because the final value for "dark" needs to be 0.000.
  //
  // Proof:
  // Dark = 0.097V x 1.7007 = 0.1650
  //        0.1650 + -0.1650 = 0.000
  // Bright = 0.685V x 1.7007 = 1.1650
  //        1.1650 + -0.1650 = 1.0000

  const float multiplier = 1.7007;
  const float offset = -0.1650;

  // Create a linear transform for calibrating the voltage input value.
  // Connect the analog input to the linear transform.

  auto input_calibration = new Linear(multiplier, offset, linear_config_path);
  analog_input->connect_to(input_calibration);

  // Create a ConfigItem for the linear transform.

  ConfigItem(input_calibration)
      ->set_title("Input Calibration")
      ->set_description("Analog input value adjustment.")
      ->set_sort_order(1100);

  // Connect the calibration output to the Signal K output.
  // This will send the calibrated value to the Signal K server
  // on the specified Signal K path. As part of
  // that output, send some metadata to indicate that the "units"
  // to be used to display this value is "ratio". Also specify that
  // the display name for this value, to be used by any Signal K
  // consumer that displays it, is "Indoor light".

  // If you want to make the SK Output path configurable, you can
  // assign the SKOutputFloat to a variable and then call
  // ConfigItem on that variable. In that case, config_path needs to be
  // defined in the constructor of the SKOutputFloat.

  input_calibration->connect_to(
      new SKOutputFloat(sk_path, "", new SKMetadata("ratio", "Indoor light")));
}

void loop() {
  event_loop()->tick();
}
