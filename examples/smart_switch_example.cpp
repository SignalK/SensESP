#include <Arduino.h>

#include "sensesp_app.h"
#include "sensesp_app_builder.h"
#include "sensors/button.h"
#include "sensors/load_controller.h"
#include "controllers/smart_switch_controller.h"
#include "signalk/signalk_listener.h"
#include "signalk/signalk_output.h"
#include "signalk/signalk_put_request_listener.h"
#include "system/led_light.h"
#include "transforms/debounce.h"
#include "transforms/click_type.h"
#include "transforms/repeat_report.h"

// This example implements a smart light switch which can control a load either via
// a manual button press, or via Signal K PUT requests.

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
  sensesp_app = builder.set_hostname("sk-engine-lights")
                    ->set_sk_server("sk-server.local", 3000)
                    ->set_wifi("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD")
                    ->set_standard_sensors(StandardSensors::NONE)
                    ->get_app();


  // Define the SK Path you want to listen to and report on
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
  const char* config_path_button_d = "/button/debounce";
  const char* config_path_button_c = "/button/clicktime";
  const char* config_path_status_light = "/button/statusLight";
  const char* config_path_sk_output = "/signalk/path";
  const char* config_path_repeat = "/signalk/repeat";



  // Create a load controller to control the light (or other electrical load), 
  // and connect it up to an LED to show the current status on the switch...
  LoadController* load_switch = new LoadController(PIN_RELAY);
  load_switch->connect_to(new LEDLight(PIN_LED_R, PIN_LED_G, PIN_LED_B, 
                                       config_path_status_light, 
                                       LED_ON_COLOR, LED_OFF_COLOR));


  // Create a switch controller to handle the user press logic and 
  // connect it to the load switch...
  SmartSwitchController* controller = new SmartSwitchController();
  controller->connect_to(load_switch);


  // Connect a physical button that will feed manual click types into the controller...
  Button* btn = new Button(PIN_BUTTON);
  btn->connect_to(new Debounce(20, config_path_button_d))
     ->connect_to(new ClickType(config_path_button_c))
     ->connect_to(controller);


  // In addition to the manual button "click types", a 
  // SmartSwitchController accepts explicit state settings via 
  // any boolean producer or various "truth" values in human readable 
  // format via a String producer.
  // Here, we set up a SignalK PUT request listener to handle
  // requests made to the Signal K to set the switch state.
  // This allows any device on the SignalK network that can make
  // such a request to also control the state of our switch.
  auto* sk_listener = new SKStringPutRequestListener(sk_path);
  sk_listener->connect_to(controller);


  // Finally, specify that the load controller should report its status at regular
  // intervals back to the SignalK server...
  load_switch->connect_to(new RepeatReport<bool>(10000, config_path_repeat))
             ->connect_to(new SKOutputBool(sk_path, config_path_sk_output));

  // Start the SensESP application running
  sensesp_app->enable();

});