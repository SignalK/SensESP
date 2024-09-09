#include "sensesp/sensors/analog_input.h"
#include "sensesp/signalk/signalk_output.h"
#include "sensesp/transforms/analogvoltage.h"
#include "sensesp/transforms/curveinterpolator.h"
#include "sensesp/transforms/linear.h"
#include "sensesp/transforms/moving_average.h"
#include "sensesp/transforms/voltagedivider.h"
#include "sensesp_app.h"
#include "sensesp_app_builder.h"

using namespace sensesp;

class ETapeInterpreter : public CurveInterpolator {
 public:
  ETapeInterpreter(String config_path = "")
      : CurveInterpolator(NULL, config_path) {
    // Populate a lookup table to translate the ohm values returned by
    // our level sensor to inches on the sensor scale
    clear_samples();
    // add_sample(CurveInterpolator::Sample(knownOhmValue, sensorHeight));
    add_sample(CurveInterpolator::Sample(388., 8.5));
    add_sample(CurveInterpolator::Sample(390., 8.3));
    add_sample(CurveInterpolator::Sample(400, 8.15));
    add_sample(CurveInterpolator::Sample(436., 8.0));
    add_sample(CurveInterpolator::Sample(457., 7.75));
    add_sample(CurveInterpolator::Sample(511., 7.25));
    add_sample(CurveInterpolator::Sample(563., 6.75));
    add_sample(CurveInterpolator::Sample(590., 6.3));
    add_sample(CurveInterpolator::Sample(654., 5.8));
    add_sample(CurveInterpolator::Sample(757., 5.15));
    add_sample(CurveInterpolator::Sample(823., 4.6));
    add_sample(CurveInterpolator::Sample(872., 4.2));
    add_sample(CurveInterpolator::Sample(953., 3.6));
    add_sample(CurveInterpolator::Sample(1036., 3.05));
    add_sample(CurveInterpolator::Sample(1112., 2.5));
    add_sample(CurveInterpolator::Sample(1155., 2.1));
    add_sample(CurveInterpolator::Sample(1221., 1.7));
    add_sample(CurveInterpolator::Sample(1329., 1.3));
    add_sample(CurveInterpolator::Sample(1387., 1.0));
    add_sample(CurveInterpolator::Sample(1600., 1.0));
  }
};

void setup() {
// Some initialization boilerplate when in debug mode...
  SetupLogging();

  // Create a builder object
  SensESPAppBuilder builder;

  // Create the global SensESPApp() object. If you add the line ->set_wifi("your
  // ssid", "your password") you can specify the wifi parameters in the builder.
  // If you do not do that, the SensESP device wifi configuration access point
  // will appear and you can use a web browser pointed to 192.168.4.1 to
  // configure the wifi parameters.

  sensesp_app = builder.set_hostname("milone")
                    ->set_sk_server("192.168.0.1", 3000)
                    ->get_app();

  // The "SignalK path" identifies this sensor to the SignalK server. Leaving
  // this blank would indicate this particular sensor (or transform) does not
  // broadcast SignalK data. This path should appear as an argument in the
  // SKOutputNumber transform.

  // The "Configuration path" is combined with "/config" to formulate a URL
  // used by the RESTful API for retrieving or setting configuration data.
  // It is ALSO used to specify a path to the SPIFFS file system
  // where configuration data is saved on the MCU board. It should
  // ALWAYS start with a forward slash if specified. If left blank,
  // that indicates this sensor or transform does not have any
  // configuration to save, or that you're not interested in doing
  // run-time configuration.

  const char *kAnalogInConfigPath = "/freshWaterTank_starboard/analogin";

  // Create a sensor that is the source of our data, that will be read every 500
  // ms. It's a Milone depth sensor that's connected to the ESP's AnalogIn pin.
  // ESP32 has many pins that can be
  // used for AnalogIn, and they're expressed here as the XX in GPIOXX.
  uint8_t pin = 36;

  unsigned int read_delay = 500;

  auto *analog_input = new AnalogInput(pin, read_delay, kAnalogInConfigPath);

  // comment out the following line to suppress the output of the raw ADC
  // measurement values. analog_input->connect_to(new
  // SKOutputNumber("tanks.freshWater.starboard.rawADC"));

  // Comment out the following 2 lines to suppress the output of the ADC
  // measurement in volts. analog_input->connect_to(new AnalogVoltage(1.0, 1.0,
  // 0))
  //             ->connect_to(new
  //             SKOutputNumber("tanks.freshWater.starboard.voltsADC"));

  // The Milone depth sensor is wired as a voltage divider with Vin connected to
  // the sensor, a variable resistor. The sensor is then connected to R2, a
  // fixed resistor then connected to gnd. The voltrage across the fixed
  // resistor is measured by the MCU analog input ADC.

  // A VoltageDeviderR1 transform is used to extract the Milone sensor
  // resistance from the data.

  const float scale = 1.0;
  const float Vin = 5.0;
  const float R2 = 100;
  const unsigned int samples = 10;

  // Wire up the output of the analog input to the VoltageDividerR1 transform.
  // and then output the results to the SignalK server.

  // Comment out the following 3 lines to suppress the output of the eTape
  // sensor resistance.
  analog_input->connect_to(new AnalogVoltage(1.0, 1.0, 0.))
      ->connect_to(
          new VoltageDividerR1(R2, Vin, "/freshWaterTank_starboard/divider"))
      ->connect_to(new SKOutputFloat("tanks.freshWater.starboard.R1"));

  // Use the ETapeInterpolator to output the water level depth in the tank and
  // pass it through the MovingAverage transport before outputting the result.

  analog_input->connect_to(new AnalogVoltage(1.0, 1.0, 0.))
      ->connect_to(new VoltageDividerR1(R2, Vin, ""))
      ->connect_to(new ETapeInterpreter(""))
      ->connect_to(new MovingAverage(samples, scale,
                                     "/freshWaterTank_starboard/samples"))
      ->connect_to(
          new SKOutputFloat("tanks.freshwater.starboard.currentLevel"));
}

void loop() {
  // We're storing the event loop in a static variable so that it's only
  // acquired once. Saves a few function calls per loop iteration.
  static auto event_loop = SensESPBaseApp::get_event_loop();
  event_loop->tick();
}
