
#include "sensesp/transforms/hysteresis.h"

#include <math.h>

#include "sensesp/sensors/analog_input.h"
#include "sensesp/signalk/signalk_output.h"
#include "sensesp_app_builder.h"

using namespace sensesp;

void setup() {
  SetupLogging();

  SensESPAppBuilder builder;
  sensesp_app = builder.set_hostname("sensesp-hysteresis-example")
                    ->set_wifi_client("My WiFi SSID", "my_wifi_password")
                    ->set_sk_server("192.168.10.3", 80)
                    ->get_app();

  const char* sk_path = "environment.indoor.illuminance";
  const char* analog_in_config_path = "/indoor_illuminance/analog_in";

  unsigned int read_delay = 500;

  uint8_t pin = 32;

  float output_scale = 3.3;

  // Use AnalogInput as an example sensor. Connect it e.g. to a photoresistor
  // or a potentiometer with a voltage divider to get an illustrative test
  // input.

  auto* analog_input =
      new AnalogInput(pin, read_delay, analog_in_config_path, output_scale);

  ConfigItem(analog_input)
      ->set_title("Analog Input")
      ->set_sort_order(1000);

  // Connect the analog input via a hysteresis transform
  // to an SKOutputBool object. The hysteresis function has arbitrary voltage
  // limits that trigger the function when moving one's hand over a
  // particular photoresistor test setup.

  auto hysteresis = new Hysteresis<float, bool>(0.3, 0.5, false, true,
                                               "/transforms/hysteresis");

  ConfigItem(hysteresis)
      ->set_title("Hysteresis")
      ->set_sort_order(1100);

  auto sk_output = new SKOutputBool(sk_path);

  ConfigItem(sk_output)
      ->set_title("SK Output")
      ->set_sort_order(1200);

  analog_input
      ->connect_to(hysteresis)
      ->connect_to(sk_output);
}

void loop() { event_loop()->tick(); }
