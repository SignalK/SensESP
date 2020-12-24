#include <Arduino.h>

#include "math.h"
#include "sensesp_app.h"
#include "sensors/analog_input.h"
#include "signalk/signalk_output.h"
#include "transforms/hysteresis.h"

ReactESP app([]() {
  SetupSerialDebug(115200);

  sensesp_app = new SensESPApp("sensesp-hysteresis-example", "My WiFi SSID",
                               "my_wifi_password", "192.168.10.3", 80);

  const char* sk_path = "environment.indoor.illuminance";
  const char* analog_in_config_path = "/indoor_illuminance/analog_in";

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

  // Connect the analog input via a hysteresis transform
  // to an SKOutputBool object. The hysteresis function has arbitrary voltage
  // limits that trigger the function when moving one's hand over a
  // particular photoresistor test setup.

  analog_input
      ->connect_to(new Hysteresis<float, bool>(0.3, 0.5, false, true,
                                               "/transforms/hysteresis"))
      ->connect_to(new SKOutputBool(sk_path));

  sensesp_app->enable();
});
