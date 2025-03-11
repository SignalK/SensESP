#include <Arduino.h>

// RPM counter example using the Pulse Counter peripheral

// #define SERIAL_DEBUG_DISABLED

#include "sensesp/sensors/digital_pcnt_input.h"
#include "sensesp/signalk/signalk_output.h"
#include "sensesp/transforms/frequency.h"
#include "sensesp_app_builder.h"

using namespace sensesp;

void setup() {
  SetupLogging();

  SensESPAppBuilder builder;
  sensesp_app = builder.get_app();

  // The "Signal K path" identifies the output of the sensor to the Signal K
  // network. If you have multiple sensors connected to your microcontoller
  // (ESP), each one of them will (probably) have its own Signal K path
  // variable. For example, if you have two propulsion engines, and you want the
  // RPM of each of them to go to Signal K, you might have sk_path_portEngine =
  // "propulsion.port.revolutions" and sk_path_starboardEngine =
  // "propulsion.starboard.revolutions" In this example, there is only one
  // propulsion engine, and its RPM is the only thing being reported to Signal
  // K. To find valid Signal K Paths that fits your need you look at this link:
  // https://signalk.org/specification/1.4.0/doc/vesselsBranch.html
  const char* sk_path = "propulsion.main.revolutions";

  // The "Configuration path" is combined with "/config" to formulate a URL
  // used by the RESTful API for retrieving or setting configuration data.
  // It is ALSO used to specify a path to the SPIFFS file system
  // where configuration data is saved on the microcontroller.  It should
  // ALWAYS start with a forward slash if specified. If left blank,
  // that indicates this sensor or transform does not have any
  // configuration to save.
  // Note that if you want to be able to change the sk_path at runtime,
  // you will need to specify a configuration path.
  // As with the Signal K path, if you have multiple configurable sensors
  // connected to the microcontroller, you will have a configuration path
  // for each of them, such as config_path_portEngine =
  // "/sensors/portEngine/rpm" and config_path_starboardEngine =
  // "/sensor/starboardEngine/rpm".
  const char* config_path = "/sensors/engine_rpm";

  // These two are necessary until a method is created to synthesize them.
  // Everything after "/sensors" in each of these ("/engine_rpm/calibrate" and
  // "/engine_rpm/sk") is simply a label to display what you're configuring in
  // the Configuration UI.
  const char* config_path_calibrate = "/sensors/engine_rpm/calibrate";
  const char* config_path_skpath = "/sensors/engine_rpm/sk";

  //////////
  // connect a RPM meter. A DigitalInputPcntCounter implements
  // access to a pulse counter peripheral. Every read_delay ms, it
  // reads the number of pulses that have occurred since the last
  // read, and reports that number (500ms in the example). A Frequency
  // transform takes a number of pulses and converts that into
  // a frequency. The sample multiplier converts the 97 tooth
  // tach output into Hz, SK native units.
  const float multiplier = 1.0 / 97.0;
  const unsigned int read_delay = 500;

  // Wire it all up by connecting the producer directly to the consumer
  // ESP32 pins are specified as just the X in GPIOX
  uint8_t pin = 4;

  auto* sensor = new DigitalInputPcntCounter(pin, INPUT_PULLUP, RISING, read_delay);

  auto frequency = new Frequency(multiplier, config_path_calibrate);

  ConfigItem(frequency)
      ->set_title("Frequency")
      ->set_description("Frequency of the engine RPM signal")
      ->set_sort_order(1000);

  auto frequency_sk_output = new SKOutput<float>(sk_path, config_path_skpath);

  ConfigItem(frequency_sk_output)
      ->set_title("Frequency SK Output Path")
      ->set_sort_order(1001);

  sensor
      ->connect_to(frequency)             // connect the output of sensor
                                          // to the input of Frequency()
      ->connect_to(frequency_sk_output);  // connect the output of Frequency()
                                          // to a Signal K Output as a number
}

void loop() { event_loop()->tick(); }
