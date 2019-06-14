#include <Arduino.h>

#include "sensesp_app.h"
#include "transforms/linear.h"
#include "sensors/analog_input.h"

#include "transforms/analogvoltage.h"
#include "transforms/voltagedividerR2.h"
#include "transforms/curveinterpolator.h"
#include "signalk/signalk_output.h"



/**
 * A custom transform that takes a resistance value on ohms and returns the estimated
 * temperature in Kelvin.  Sample data in this example were taken from a Westerbeke generator
 * temperature gauge.
 */
class TemperatureInterpreter : public CurveInterpolator {

    public:
        TemperatureInterpreter(String config_path="") :
           CurveInterpolator(NULL, config_path ) {

          // Populate a lookup table tp translate the ohm values returned by
          // our temperature sender to Kelvin
          clearSamples();
//          addSample(CurveInterpolator::Sample(knownOhmValue, knopwnKelvin));
          addSample(CurveInterpolator::Sample(0, 418.9));
          addSample(CurveInterpolator::Sample(5, 414.71));
          addSample(CurveInterpolator::Sample(36, 388.71));
          addSample(CurveInterpolator::Sample(56, 371.93));
          addSample(CurveInterpolator::Sample(59, 366.48));
          addSample(CurveInterpolator::Sample(81, 355.37));
          addSample(CurveInterpolator::Sample(112, 344.26));
          addSample(CurveInterpolator::Sample(240, 322.04));
          addSample(CurveInterpolator::Sample(550, 255.37));
          addSample(CurveInterpolator::Sample(10000, 237.6));

        }
};



// SensESP builds upon the ReactESP framework. Every ReactESP application
// defines an "app" object vs defining a "main()" method.
ReactESP app([] () {

  // Some initialization boilerplate when in debug mode...
  #ifndef SERIAL_DEBUG_DISABLED
  Serial.begin(115200);

  // A small arbitrary delay is required to let the
  // serial port catch up

  delay(1000);
  Debug.setSerialEnabled(true);
  #endif

  // Create the global SensESPApp() object.
  sensesp_app = new SensESPApp();


  // The "SignalK path" identifies this sensor to the SignalK network.
  const char* sk_path = "electrical.generator.engine.water.temp";



  // Connecting a "sender" to the MCU involves using a "voltage divider" circuit. A resistor (R1) is connected
  // to the voltage output of the board (3.3v or 5v, specified in Vin) to an intersection of two wires.  One
  // wire connects to the AnalogInput pin of the MCU, and the other wire connects to the sending
  // unit doing the measurement. The sender is itself a variable resistor that is then connected to ground, forming
  // the "R2" of a voltage divider circuit. The transform "VoltageDividerR2" can take a known
  // voltage in, resistance R1, and based on the current voltage read from AnalogInput, compute
  // the current resistance of the sender (R2).
  const float Vin = 3.3; // Voltage sent into the voltage divider circuit that includes the analog sender
  const float R1 = 51.0; // The resistance, in ohms, of the R1 resitor in the analog sender divider circuit

  auto* pAnalogInput = new AnalogInput();

  // Translating the number returned by AnalogInput into a temperature requires several transformations.
  // Wire them up in sequence...
  pAnalogInput->connectTo(new AnalogVoltage()) ->
                connectTo(new VoltageDividerR2(R1, Vin, "/gen/temp/sender")) ->
                connectTo(new TemperatureInterpreter("/gen/temp/curve")) ->
                connectTo(new Linear(1.0, 0.0, "/gen/temp/calibrate")) ->
                connectTo(new SKOutputNumber(sk_path, "/gen/temp/sk"));


  // Start the SensESP application running
  sensesp_app->enable();

});
