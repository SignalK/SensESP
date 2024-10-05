/**
 * @file repeat_sensor_analog_input.cpp
 * @brief Example of a RepeatSensor that reads an analog input.
 *
 * RepeatSensor is a helper class that can be used to wrap any generic Arduino
 * sensor library into a SensESP sensor. This file demonstrates its use with
 * the built-in analog input sensor.
 *
 * Note that for analog input, it's actually better to use the AnalogInput
 * sensor class as demonstrated by the analog_input.cpp example file. The
 * AnalogInput class compensates for the ESP32 ADC nonlinearities. The approach
 * presented here, however, can be used with any sensor library compatible with
 * arduino-esp32.
 *
 */

#include "sensesp/signalk/signalk_output.h"
#include "sensesp/transforms/linear.h"
#include "sensesp_app_builder.h"

using namespace sensesp;

// GPIO pin that we'll be using for the analog input.
const uint8_t kAnalogInputPin = 36;

// define a callback function that reads the analog input
float analog_read_callback() { return analogRead(kAnalogInputPin) / 4096.0; }

// The setup function performs one-time application initialization.
void setup() {
  SetupLogging();

  // Create the global SensESPApp() object.
  SensESPAppBuilder builder;
  sensesp_app = builder.get_app();

  // Do here any sensor initialization you need. For a more complex sensor,
  // this might include initializing the hardware, setting the I2C pins, etc.
  // In this case, we'll just set the analog input attenuation to 2.5 dB,
  // corresponding to maximum input voltage of 1.1 V.
  analogSetAttenuation(ADC_2_5db);

  // Let's read the sensor every 100 ms.
  unsigned int read_interval = 100;

  // Create a RepeatSensor with float output that reads the analog input.
  auto* analog_input =
      new RepeatSensor<float>(read_interval, analog_read_callback);

  // The "Signal K path" identifies this sensor to the Signal K server.
  //
  // To find valid Signal K Paths that fits your need you look at this link:
  // https://signalk.org/specification/1.4.0/doc/vesselsBranch.html
  const char* sk_path = "environment.indoor.illuminance";

  // Connect the output of the analog input to the SKOutput object which
  // transmits the results to the Signal K server. As part of
  // that output, send some metadata to indicate that the "units"
  // to be used to display this value is "ratio". Also specify that
  // the display name for this value, to be used by any Signal K
  // consumer that displays it, is "Indoor light".
  analog_input->connect_to(
      new SKOutputFloat(sk_path, "", new SKMetadata("ratio", "Indoor light")));
}

void loop() { event_loop()->tick(); }
