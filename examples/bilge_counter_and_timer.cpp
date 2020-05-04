#include <Arduino.h>

#include "sensesp_app.h"
#include "sensors/digital_input.h"
#include "transforms/debounce.h"
#include "transforms/counter.h"
#include "transforms/timer.h"
#include "signalk/signalk_output.h"

// SensESP builds upon the ReactESP framework. Every ReactESP application
// defines an "app" object vs defining a "main()" method.
ReactESP app([] () {

  // Some initialization boilerplate when in debug mode...
  #ifndef SERIAL_DEBUG_DISABLED
  Serial.begin(115200);

  // A small arbitrary delay is required to let the
  // serial port catch up
  delay(100);
  Debug.setSerialEnabled(true);
  #endif


  // Create the global SensESPApp() object.
  sensesp_app = new SensESPApp();


  // The "SignalK path" identifies this sensor to the SignalK server. Leaving
  // this blank would indicate this particular sensor (or transform) does not
  // broadcast SignalK data.
  const char* engine_counter_path = "bilge.engine.counter";
  const char* engine_timer_path = "bilge.engine.timer";
  const char* cabin_counter_path = "bilge.cabin.counter";
  const char* cabin_timer_path = "bilge.cabin.timer";
  const char* midship_counter_path = "bilge.midship.counter";
  const char* midship_timer_path = "bilge.midship.timer";


  // The "Configuration path" is combined with "/config" to formulate a URL
  // used by the RESTful API for retrieving or setting configuration data.
  // It is ALSO used to specify a path to the SPIFFS file system
  // where configuration data is saved on the MCU board. It should
  // ALWAYS start with a forward slash if specified. If left blank,
  // that indicates this sensor or transform does not have any
  // configuration to save, or that you're not interested in doing
  // run-time configuration.
  const char* counter_engine_config_path = "/bilges/engine_counter";
  const char* timer_engine_config_path = "/bilges/engine_timer";

  const char* counter_cabin_config_path = "/bilges/cabin_counter";
  const char* timer_cabin_config_path = "/bilges/cabin_timer";
    
  const char* counter_midship_config_path = "/bilges/midship_counter";
  const char* timer_midship_config_path = "/bilges/midship_counter";
  
  // Create a sensor that is the source of our data. 
  // The digital pins from the ESP are wired to LEDs that show when
  // the bilge is running. When the LED is on that means the bilge pump is
  // running.
  uint8_t enginePin = 4;
  uint8_t cabinPin = 5;
  uint8_t midshipPin = 13;

  // Constant for the Timer and Counter for when it will reset in milliseconds
  const unsigned long oneDay = 1440 * 60 * 1000UL;

  auto* pEngineInput = new DigitalInputValue(enginePin, INPUT_PULLDOWN, RISING);  
  auto* pCabinInput = new DigitalInputValue(cabinPin, INPUT_PULLUP, RISING);  
  auto* pMidshipInput = new DigitalInputValue(midshipPin, INPUT_PULLUP, RISING);  

  // Since the input pins are in pulldown the default value of the pin is true,
  // timer and counter has to be configured accordingly
  pEngineInput->connectTo(new Debounce())
              ->connectTo(new Timer(oneDay, true, timer_engine_config_path))
              ->connectTo(new SKOutputInt(engine_timer_path));

  pEngineInput->connectTo(new Debounce()) 
              ->connectTo(new Counter(oneDay, true, counter_engine_config_path))
              ->connectTo(new SKOutputInt(engine_counter_path));


  pCabinInput->connectTo(new Debounce())
             ->connectTo(new Timer(oneDay, true, timer_cabin_config_path))
             ->connectTo(new SKOutputInt(cabin_timer_path));

  pCabinInput->connectTo(new Debounce()) 
             ->connectTo(new Counter(oneDay, true, counter_cabin_config_path))
             ->connectTo(new SKOutputInt(cabin_counter_path));

  pMidshipInput->connectTo(new Debounce())
               ->connectTo(new Timer(oneDay, true, timer_midship_config_path))
               ->connectTo(new SKOutputInt(midship_timer_path));

  pMidshipInput->connectTo(new Debounce()) 
               ->connectTo(new Counter(oneDay, true, counter_midship_config_path))
               ->connectTo(new SKOutputInt(midship_counter_path));

  // Start the SensESP application running
  sensesp_app->enable();
});