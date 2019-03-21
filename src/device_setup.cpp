#include "app.h"
#include "devices/analog_input.h"
#include "devices/digital_input.h"
#include "devices/onewire_temperature.h"

void SensESPApp::setup_custom_devices() {

  // connect an analog input

  connect_1to1<AnalogInput, Passthrough<float>>(
    new AnalogInput(),
    new Passthrough<float>("sensors.indoor.illumination"));

  // expose a slowly changing (button-type) digital input

  DigitalInputValue* digin = new DigitalInputValue(D1, INPUT_PULLUP, RISING);
  connect_1to1<DigitalInputValue, Passthrough<bool>>(
    digin,
    new Passthrough<bool>("sensors.sensesp.button")
  );

#if 0
  // FIXME: Difference transform is still missing

  // connect a fuel flow meter with return line

  connect_1to1<DigitalInputCounter, Frequency>(
    new DigitalInputCounter(D1, INPUT_PULLUP, CHANGE, 1000),
    new Frequency("sensors.inflow.frequency")
  );

  connect_1to1<DigitalInputCounter, Frequency>(
    new DigitalInputCounter(D2, INPUT_PULLUP, CHANGE, 1000),
    new Frequency("sensors.outflow.frequency")
  );
#endif

  // connect a RPM meter. A DigitalInputCounter counts pulses
  // and reports the readings every read_delay ms
  // (500 in the example). A Frequency
  // transform takes a number of pulses and converts that into
  // a frequency. The sample multiplier converts the 97 tooth
  // tach output into Hz, SK native units.

  connect_1to1<DigitalInputCounter, Frequency>(
    new DigitalInputCounter(D5, INPUT_PULLUP, RISING, 500),
    new Frequency("sensors.engine.rpm", 1./97.)
  );

#if 0
  // OneWire temperature devices need to have the bus
  // controller device created first
  DallasTemperatureSensors* dts = new DallasTemperatureSensors(13);

  connect_1to1<OneWireTemperature, Passthrough<float>>(
    new OneWireTemperature(dts, "/devices/ow_temp_1", ""),
    new Passthrough<float>("sensors.temperature.ow1",
                           "/transforms/ow_temp_1"));
#endif
}
