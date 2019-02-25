#include "app.h"
#include "devices/analog_input.h"
#include "devices/digital_input.h"
#include "devices/onewire_temperature.h"

void SensESPApp::setup_custom_devices() {
  // connect an analog input

  connect_1to1<AnalogInput, Passthrough<float>>(
    new AnalogInput(),
    new Passthrough<float>("sensors.indoor.illumination"));

  // connect a digital input to two different transforms

  DigitalInput* digin = new DigitalInput(D1, INPUT_PULLUP, CHANGE);
  // connect_1to1<DigitalInput, Passthrough<bool>>(
  //   digin,
  //   new Passthrough<bool>("sensors.sensesp.button")
  // );

  // TODO FIXME

  // connect a fuel flow meter with return line

  // connect_1to1<DigitalInput, Frequency<bool>>(
    // digin,
    // new Frequency<bool>("sensors.inflow.frequency")
  // );
//
  // connect_1to1<DigitalInput, Frequency<bool>>(
    // new DigitalInput(D2, INPUT_PULLUP, CHANGE),
    // new Frequency<bool>("sensors.outflow.frequency")
  // );
//
  // connect a RPM meter

  connect_1to1<DigitalInput, Frequency<bool>>(
    new DigitalInput(D5, INPUT_PULLUP, CHANGE),
    new Frequency<bool>("sensors.engine.rpm")
  );

  // OneWire temperature devices need to have the bus
  // controller device created first
  // DallasTemperatureSensors* dts = new DallasTemperatureSensors(13);
  // devices.insert(dts);

  // connect_1to1<OneWireTemperature, Passthrough<float>>(
  //   new OneWireTemperature(dts, "/devices/ow_temp_1", ""),
  //   new Passthrough<float>("sensors.temperature.ow1",
  //                          "/transforms/ow_temp_1"));

}
