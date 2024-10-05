#include <Arduino.h>

#include "sensesp/controllers/smart_switch_controller.h"
#include "sensesp/sensors/digital_input.h"
#include "sensesp/sensors/digital_output.h"
#include "sensesp/signalk/signalk_listener.h"
#include "sensesp/signalk/signalk_output.h"
#include "sensesp/signalk/signalk_put_request_listener.h"
#include "sensesp/system/rgb_led.h"
#include "sensesp/transforms/click_type.h"
#include "sensesp/transforms/debounce.h"
#include "sensesp/transforms/press_repeater.h"
#include "sensesp/transforms/repeat_report.h"
#include "sensesp_app.h"
#include "sensesp_app_builder.h"

using namespace sensesp;

// This example implements a smart light switch which can control a load either
// via a manual button press, or via Signal K PUT requests.

// Control devices are wired to the following digital pins

#ifdef ESP32
#define PIN_LED_R 6
#define PIN_LED_G 7
#define PIN_LED_B 8
#define PIN_BUTTON 0
#define PIN_RELAY 5
#else
#define PIN_LED_R D6
#define PIN_LED_G D7
#define PIN_LED_B D8
#define PIN_BUTTON D0
#define PIN_RELAY D5
#endif

#define LED_ON_COLOR 0x004700
#define LED_OFF_COLOR 0x261900

void setup() {
  SetupLogging();

  // Create a builder object
  SensESPAppBuilder builder;

  // Create the global SensESPApp() object.
  sensesp_app = builder.set_hostname("sk-engine-lights")
                    ->set_sk_server("192.168.10.3", 3000)
                    ->set_wifi("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD")
                    ->get_app();

  // Define the SK Path that represents the load this device controls.
  // This device will report its status on this path, as well as
  // respond to PUT requests to change its status.
  // To find valid Signal K Paths that fits your need you look at this link:
  // https://signalk.org/specification/1.4.0/doc/vesselsBranch.html
  const char* sk_path = "electrical.switches.lights.engineroom.state";

  // "Configuration paths" are combined with "/config" to formulate a URL
  // used by the RESTful API for retrieving or setting configuration data.
  // It is ALSO used to specify a path to the SPIFFS file system
  // where configuration data is saved on the MCU board.  It should
  // ALWAYS start with a forward slash if specified.  If left blank,
  // that indicates a sensor or transform does not have any
  // configuration to save.
  const char* config_path_button_c = "/button/clicktime";
  const char* config_path_status_light = "/button/statusLight";
  const char* config_path_sk_output = "/signalk/path";

  // Create a digital output that is assumed to be connected to the
  // control channel of a relay or a MOSFET that will control the
  // electric light.  Also connect this pin's state to an LED to get
  // a visual indicator of load's state.
  auto* load_switch = new DigitalOutput(PIN_RELAY);
  load_switch->connect_to(new RgbLed(PIN_LED_R, PIN_LED_G, PIN_LED_B,
                                     config_path_status_light, LED_ON_COLOR,
                                     LED_OFF_COLOR));

  // Create a switch controller to handle the user press logic and
  // connect it to the load switch...
  SmartSwitchController* controller = new SmartSwitchController();
  controller->connect_to(load_switch);

  // Connect a physical button that will feed manual click types into the
  // controller...
  DigitalInputState* btn = new DigitalInputState(PIN_BUTTON, INPUT, 100);
  PressRepeater* pr = new PressRepeater();
  btn->connect_to(pr);
  pr->connect_to(new ClickType(config_path_button_c))->connect_to(controller);

  // In addition to the manual button "click types", a
  // SmartSwitchController accepts explicit state settings via
  // any boolean producer as well as any "truth" values in human readable
  // format via a String producer.
  // Here, we set up a SignalK PUT request listener to handle
  // requests made to the Signal K server to set the switch state.
  // This allows any device on the SignalK network that can make
  // such a request to also control the state of our switch.
  auto* sk_listener = new StringSKPutRequestListener(sk_path);
  sk_listener->connect_to(controller);

  // Finally, connect the load switch to an SKOutput so it reports its state
  // to the Signal K server.  Since the load switch only reports its state
  // whenever it changes (and switches like light switches change infrequently),
  // send it through a `RepeatReport` transform, which will cause the state
  // to be reported to the server every 10 seconds, regardless of whether
  // or not it has changed.  That keeps the value on the server fresh and
  // lets the server know the switch is still alive.
  load_switch->connect_to(new Repeat<bool, bool>(10000))
      ->connect_to(new SKOutputBool(sk_path, config_path_sk_output));
}

void loop() { event_loop()->tick(); }
