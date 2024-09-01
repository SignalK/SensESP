#include "sensesp/transforms/lambda_transform.h"

#include <math.h>

#include "sensesp/sensors/analog_input.h"
#include "sensesp/signalk/signalk_output.h"
#include "sensesp_app_builder.h"

using namespace sensesp;

reactesp::ReactESP app;

void setup() {
  SetupLogging();

  // Create a new SensESPApp object. This is the direct constructor call, and
  // an equivalent alternative to using the SensESPAppBuilder class.

  SensESPAppBuilder builder;

  sensesp_app = builder.set_hostname("sensesp-illum-example")
                    ->set_wifi("My WiFi SSID", "my_wifi_password")
                    ->set_sk_server("192.168.1.13", 80)
                    ->get_app();

  // To find valid Signal K Paths that fits your need you look at this link:
  // https://signalk.org/specification/1.4.0/doc/vesselsBranch.html
  const char* sk_path = "environment.indoor.illuminance";
  const char* analog_in_config_path = "/Sensors/Analog Input";

  unsigned int read_delay = 500;

  uint8_t pin = 32;

  float output_scale = 3.3;

  // Use AnalogInput as an example sensor. Connect it e.g. to a photoresistor
  // or a potentiometer with a voltage divider to get an illustrative test
  // input.

  auto* analog_input =
      new AnalogInput(pin, read_delay, analog_in_config_path, output_scale);

  analog_input->set_description(
      "Connect the analog input at pin 32 to a "
      "photoresistor or potentiometer with a voltage "
      "divider to get an illustrative test input.");

  // This is our transform function. The example is artificial; a log transform
  // with configurable multiplier, base, and offset parameters. The
  // final "-> float" refers to the return type of the function.

  auto log_function = [](float input, float multiplier, float base,
                         float offset) -> float {
    return multiplier * log(input) / log(base) + offset;
  };

  // This is an array of parameter information, providing the keys and
  // descriptions required to display user-friendly values in the configuration
  // interface.

  const ParamInfo* log_lambda_param_data = new ParamInfo[3]{
      {"multiplier", "Multiplier"}, {"base", "Base"}, {"offset", "Offset"}};

  // Here we create a new LambdaTransform objects. The template parameters
  // (five floats in this example) correspond to the following types:
  //
  // 1. Input type of the transform function
  // 2. Output type of the transform function
  // 3. Type of parameter 1
  // 4. Type of parameter 2
  // 5. Type of parameter 3
  //
  // The function arguments are:
  // 1. The tranform function
  // 2-4. Default values for parameters
  // 5. Parameter data for the web config UI
  // 6. Configuration path for the transform

  auto log_transform = new LambdaTransform<float, float, float, float, float>(
      log_function, 10, 2, 100, log_lambda_param_data,
      "/Transforms/Log Transform");

  log_transform->set_description(
      "Log transform performs a configurable logarithmic "
      "transform on the input.");

  // Finally, connect the analog input via the freshly-generated log_transform
  // to an SKOutputNumber object

  analog_input->connect_to(log_transform)
      ->connect_to(new SKOutputFloat(sk_path));
}

// The loop function is called in an endless loop during program execution.
// It simply calls `app.tick()` which will then execute all reactions as needed.
void loop() { app.tick(); }
