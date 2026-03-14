#include "sensesp/sensors/sensor.h"
#include "sensesp/signalk/signalk_output.h"
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

  // Create the global SensESPApp() object. If you add the line ->set_wifi_client("your
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

  // Create a sensor that is the source of our data, that will be read every 500
  // ms. It's a Milone depth sensor that's connected to the ESP's analog input
  // pin. ESP32 has many pins that can be used for analog input, and they're
  // expressed here as the XX in GPIOXX.
  const uint8_t pin = 36;

  unsigned int read_delay = 500;

  auto* analog_input = new RepeatSensor<float>(read_delay, [pin]() {
    return analogReadMilliVolts(pin) / 1000.;
  });

  // The Milone depth sensor is wired as a voltage divider with Vin connected to
  // the sensor, a variable resistor. The sensor is then connected to R2, a
  // fixed resistor then connected to gnd. The voltage across the fixed
  // resistor is measured by the MCU analog input ADC.

  // A VoltageDividerR1 transform is used to extract the Milone sensor
  // resistance from the data.

  const float scale = 1.0;
  const float Vin = 5.0;
  const float R2 = 100;
  const unsigned int samples = 10;

  // Wire up the output of the analog input to the VoltageDividerR1 transform
  // and then output the results to the SignalK server.
  // The RepeatSensor outputs voltage directly, so no AnalogVoltage conversion
  // is needed.

  auto voltage_divider = new VoltageDividerR1(R2, Vin, "/freshWaterTank_starboard/divider");

  ConfigItem(voltage_divider)
      ->set_title("Voltage Divider")
      ->set_sort_order(1100);

  analog_input->connect_to(voltage_divider)
      ->connect_to(new SKOutputFloat("tanks.freshWater.starboard.R1"));

  // Use the ETapeInterpolator to output the water level depth in the tank and
  // pass it through the MovingAverage transport before outputting the result.

  auto voltage_divider2 = new VoltageDividerR1(R2, Vin, "/freshWaterTank_starboard/divider2");

  ConfigItem(voltage_divider2)
      ->set_title("Voltage Divider 2")
      ->set_sort_order(1300);

  auto moving_average = new MovingAverage(samples, scale, "/freshWaterTank_starboard/samples");

  analog_input->connect_to(voltage_divider2)
      ->connect_to(new ETapeInterpreter(""))
      ->connect_to(moving_average)
      ->connect_to(
          new SKOutputFloat("tanks.freshwater.starboard.currentLevel"));
}

void loop() { event_loop()->tick(); }
