#include "app.h"
#include "devices/analog_input.h"
#include "devices/digital_input.h"

void SensESPApp::setup_custom_devices() {
  // connect an analog input

  connect_1to1<AnalogInput, Passthrough<float>>(
    new AnalogInput(),
    new Passthrough<float>("sensors.indoor.illumination"));

  // connect a digital input to two different transforms

  DigitalInput* digin = new DigitalInput(D1, INPUT_PULLUP, CHANGE);
  connect_1to1<DigitalInput, Passthrough<bool>>(
    digin,
    new Passthrough<bool>("sensors.sensesp.button")
  );

  connect_1to1<DigitalInput, Frequency<bool>>(
    digin,
    new Frequency<bool>("sensors.button.frequency")
  );
}
