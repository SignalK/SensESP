#include <Arduino.h>
#include <EEPROM.h> // Should not be necessary

#include "sensesp_app.h"
#include "sensesp_app_builder.h"
#include "sensors/analog_input.h"
#include "transforms/linear.h"
#include "signalk/signalk_output.h"
#include "transforms/moving_average.h"
#include "transforms/VoltageDivider.h"
#include "transforms/AnalogVoltage.h"
#include "transforms/CurveInterpolator.h"

class ETapeInterpreter : public CurveInterpolator
{

public:
  ETapeInterpreter(String config_path = "") : CurveInterpolator(NULL, config_path)
  {

    // Populate a lookup table tp translate the ohm values returned by
    // our level sensor to inches on the sensor scale
    clearSamples();
    // addSample(CurveInterpolator::Sample(knownOhmValue, sensorHeight));
    addSample(CurveInterpolator::Sample(388., 8.5));
    addSample(CurveInterpolator::Sample(390., 8.3));
    addSample(CurveInterpolator::Sample(400, 8.15));
    addSample(CurveInterpolator::Sample(436., 8.0));
    addSample(CurveInterpolator::Sample(457., 7.75));
    addSample(CurveInterpolator::Sample(511., 7.25));
    addSample(CurveInterpolator::Sample(563., 6.75));
    addSample(CurveInterpolator::Sample(590., 6.3));
    addSample(CurveInterpolator::Sample(654., 5.8));
    addSample(CurveInterpolator::Sample(757., 5.15));
    addSample(CurveInterpolator::Sample(823., 4.6));
    addSample(CurveInterpolator::Sample(872., 4.2));
    addSample(CurveInterpolator::Sample(953., 3.6));
    addSample(CurveInterpolator::Sample(1036., 3.05));
    addSample(CurveInterpolator::Sample(1112., 2.5));
    addSample(CurveInterpolator::Sample(1155., 2.1));
    addSample(CurveInterpolator::Sample(1221., 1.7));
    addSample(CurveInterpolator::Sample(1329., 1.3));
    addSample(CurveInterpolator::Sample(1387., 1.0));
    addSample(CurveInterpolator::Sample(1600., 1.0));
  }
};

// SensESP builds upon the ReactESP framework. Every ReactESP application
// defines an "app" object vs defining a "main()" method.
ReactESP app([]() {

// Some initialization boilerplate when in debug mode...
#ifndef SERIAL_DEBUG_DISABLED
  Serial.begin(115200);

  // A small delay and one debugI() are required so that
  // the serial output displays everything
  delay(100);
  Debug.setSerialEnabled(true);
#endif
  delay(100);
  debugI("Serial debug enabled");

  // Create a builder object
  SensESPAppBuilder builder;

  // Create the global SensESPApp() object. If you add the line ->set_wifi("your ssid", "your password") you can specify
  // the wifi parameters in the builder. If you do not do that, the SensESP device wifi configuration hotspot will appear and you can use a web
  // browser pointed to 192.168.4.1 to configure the wifi parameters.

  sensesp_app = builder.set_hostname("milone")
                    ->set_standard_sensors(IP_ADDRESS)
                    ->set_sk_server("192.168.0.1", 3000)
                    ->get_app();

  // The "SignalK path" identifies this sensor to the SignalK server. Leaving
  // this blank would indicate this particular sensor (or transform) does not
  // broadcast SignalK data. This path should appear as an argument in the SKOutputNumber transform.

  // The "Configuration path" is combined with "/config" to formulate a URL
  // used by the RESTful API for retrieving or setting configuration data.
  // It is ALSO used to specify a path to the SPIFFS file system
  // where configuration data is saved on the MCU board. It should
  // ALWAYS start with a forward slash if specified. If left blank,
  // that indicates this sensor or transform does not have any
  // configuration to save, or that you're not interested in doing
  // run-time configuration.

  const char *kAnalogInConfigPath = "/freshWaterTank_starboard/analogin";

  // Create a sensor that is the source of our data, that will be read every 500 ms.
  // It's a Milone depth sensor that's connected to the ESP's AnalogIn pin.
  // The AnalogIn pin on ESP8266 is always A0, but ESP32 has many pins that can be
  // used for AnalogIn, and they're expressed here as the XX in GPIOXX.

  uint8_t pin = A0;
  uint read_delay = 500;

  auto *pAnalogInput = new AnalogInput(pin, read_delay, kAnalogInConfigPath);

  // comment out the following line to suppress the output of the raw ADC measurement values.
  // pAnalogInput->connectTo(new SKOutputNumber("tanks.freshWater.starboard.rawADC"));

  // Comment out the following 2 lines to suppress the output of the ADC measurement in volts.
  // pAnalogInput->connectTo(new AnalogVoltage(1.0, 1.0, 0))
  //             ->connectTo(new SKOutputNumber("tanks.freshWater.starboard.voltsADC"));

  // The Milone depth sensor is wired as a voltage divider with Vin connected to the sensor,
  // a variable resistor. The sensor is then connected to R2, a fixed resistor then connected to gnd.
  // The voltrage across the fixed resistor is measured by the MCU analog input ADC.

  // A VoltageDeviderR1 transform is used to extract the Milone sensor resistance from the data.

  const float scale = 1.0;
  const float Vin = 5.0;
  const float R2 = 100;
  const uint samples = 10;

  // Wire up the output of the analog input to the VoltageDividerR1 transform.
  // and then output the results to the SignalK server.

  // Comment out the following 3 lines to suppress the output of the eTape sensor resistance.
  pAnalogInput->connectTo(new AnalogVoltage(1.0, 1.0, 0.))
      ->connectTo(new VoltageDividerR1(R2, Vin, "/freshWaterTank_starboard/divider"))
      ->connectTo(new SKOutputNumber("tanks.freshWater.starboard.R1"));

  // Use the ETapeInterpolator to output the water level depth in the tank and pass it through
  // the MovingAverage transport before outputting the result.

  pAnalogInput->connectTo(new AnalogVoltage(1.0, 1.0, 0.))
      ->connectTo(new VoltageDividerR1(R2, Vin, ""))
      ->connectTo(new ETapeInterpreter(""))
      ->connectTo(new MovingAverage(samples, scale, "/freshWaterTank_starboard/samples"))
      ->connectTo(new SKOutputNumber("tanks.freshwater.starboard.currentLevel"));

  // Start the SensESP application running
  sensesp_app->enable();
});
