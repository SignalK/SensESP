#include <Arduino.h>

#include "sensesp_app.h"
#include "sensesp_app_builder.h"
#include "sensors/digital_input.h"
#include "sensors/digital_output.h"
#include "controllers/smart_switch_controller.h"
#include "signalk/signalk_listener.h"
#include "signalk/signalk_output.h"
#include "signalk/signalk_put_request.h"
#include "signalk/signalk_value_listener.h"
#include "system/rgb_led.h"
#include "transforms/debounce.h"
#include "transforms/click_type.h"
#include "transforms/repeat_report.h"
#include "transforms/press_repeater.h"

// This example implements a remote switch that can remotely control the
// smart switch created in smart_switch_example.cpp 

#ifdef ESP32
#define PIN_LED_R 6
#define PIN_LED_G 7
#define PIN_LED_B 8
#define PIN_BUTTON 0
#else
#define PIN_LED_R D6
#define PIN_LED_G D7
#define PIN_LED_B D8
#define PIN_BUTTON D0
#endif

#define LED_ON_COLOR 0x004700
#define LED_OFF_COLOR 0x261900


// SensESP builds upon the ReactESP framework. Every ReactESP application
// defines an "app" object (vs. defining a "main()" method).
ReactESP app([]() {

// Some initialization boilerplate when in debug mode...
#ifndef SERIAL_DEBUG_DISABLED
  SetupSerialDebug(115200);
#endif

  // Create a builder object
  SensESPAppBuilder builder;

  // Create the global SensESPApp() object.
  sensesp_app = builder.set_hostname("sk-lights-vswitch")
                    ->set_sk_server("10.10.1.5", 3000)
                    ->set_wifi("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD")
                    ->get_app();


  // Define the SK Path of the device that controls the load that this
  // switch should control remotely.  Clicking the virtual switch will
  // send a PUT request on this path to the main device. This virtual
  // switch will also subscribe to this path, and will set its state
  // internally to match any value reported on this path.
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
  const char* config_path_repeat = "/signalk/repeat";


  // An led that represents the current state of the switch.
  auto* led = new RgbLed(PIN_LED_R, PIN_LED_G, PIN_LED_B, 
                         config_path_status_light, 
                         LED_ON_COLOR, LED_OFF_COLOR);


  // Create a switch controller to handle the user press logic and 
  // connect it a server PUT request...
  SmartSwitchController* controller = new SmartSwitchController(false);
  controller->connect_to(new BoolSKPutRequest(sk_path));

  // Also connect the controller to an onboard LED...
  controller->connect_to(led);


  // Connect a physical button that will feed manual click types into the controller...
  DigitalInputState* btn = new DigitalInputState(PIN_BUTTON, INPUT, CHANGE, 100);
  PressRepeater* pr = new PressRepeater();
  btn->connect_to(pr);
  pr->connect_to(new ClickType(config_path_button_c))
     ->connect_to(controller);


  // In addition to the manual button "click types", a 
  // SmartSwitchController accepts explicit state settings via 
  // any boolean producer or various "truth" values in human readable 
  // format via a String producer.
  // Since this device is NOT the device that directly controls the
  // load, we don't want to respond to PUT requests. Instead, we
  // let the "real" switch respond to the PUT requests, and
  // here we just listen to the published values that are
  // sent across the Signal K network when the controlling device 
  // confirms it has made the change in state.
  auto* sk_listener = new SKValueListener<bool>(sk_path);
  sk_listener->connect_to(controller);


  // Start the SensESP application running
  sensesp_app->enable();

});