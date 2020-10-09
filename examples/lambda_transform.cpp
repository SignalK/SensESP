#include <Arduino.h>

#include "math.h"
#include "sensesp_app.h"
#include "sensors/analog_input.h"
#include "signalk/signalk_output.h"
#include "transforms/lambda_transform.h"

ReactESP app([]() {
  SetupSerialDebug(115200);

  sensesp_app = new SensESPApp("sensesp-illum-example", "My WiFi SSID",
                               "my_wifi_password", "skdev.lan", 80, NONE);

  const char* sk_path = "indoor.illumination";
  const char* analog_in_config_path = "/indoor_illumination/analog_in";

  uint read_delay = 500;

#ifdef ESP8266
  uint8_t pin = A0;
#elif defined(ESP32)
  uint8_t pin = 32;
#endif

  float output_scale = 3.3;

  // Use AnalogInput as an example sensor. Connect it e.g. to a photoresistor
  // or a potentiometer with a voltage divider to get an illustrative test
  // input.

  auto* analog_input =
      new AnalogInput(pin, read_delay, analog_in_config_path, output_scale);

  // This is our transform function. The example is artificial; a log transform
  // with configurable multiplier, base, and offset parameters.

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
  // (four floats in this example) correspond to the following types:
  // 1. Output type of the transform function
  // 2. Input type of the transoform function
  // 3. Type of parameter 1
  // 4. Type of parameter 2

  auto log_transform = new LambdaTransform<float, float, float, float>(
      log_function, 10, 2, 100, log_lambda_arg_data, "/transforms/log");
      log_function, 10, 2, 100, log_lambda_param_data, "/transforms/log");

  // Finally, connect the analog input via the freshly-generated log_transform
  // to an SKOutputNumber object

  analog_input->connect_to(log_transform)
      ->connect_to(new SKOutputNumber(sk_path));

  sensesp_app->enable();
});
