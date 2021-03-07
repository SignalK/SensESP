// multiple_sensors_example.cpp

/**
 * This example illustrates only a single concept - how to combine multiple sensors
 * into a single main.cpp. (All the other examples show only a single sensor.)
 * It combines the code from two of the other examples: sht31_example.cpp and
 * rpm_counter.cpp.
 * 
 * All of the original comments have been removed from those two examples, and new
 * comments have been added to show how multiple sensors can be read. For the
 * detailed comments about either of the two examples included here, see their
 * respective example file.
 **/ 

#include <Arduino.h>

#define USE_LIB_WEBSOCKET true

#include "sensesp_app.h"
#include "sensors/sht31.h"
#include "sensors/digital_input.h"
#include "transforms/frequency.h"
#include "signalk/signalk_output.h"

ReactESP app([]() {
#ifndef SERIAL_DEBUG_DISABLED
  SetupSerialDebug(115200);
#endif

  sensesp_app = new SensESPApp();

  // Everything above this line is boilerplate, except for the #include statements

  // This is the beginning of the code to read the SHT31 temperature and humidity
  // sensor.

  auto* sht31 = new SHT31(0x45);
  const uint sht31_read_delay = 1000;

  auto* sht31_temperature =
      new SHT31Value(sht31, SHT31Value::temperature, sht31_read_delay, "/fridge/temperature");
  sht31_temperature->connect_to(
      new SKOutputNumber("environment.inside.refrigerator.temperature"));

  auto* sht31_humidity =
      new SHT31Value(sht31, SHT31Value::humidity, sht31_read_delay, "/fridge/humidity");
  sht31_humidity->connect_to(
      new SKOutputNumber("environment.inside.refrigerator.humidity"));

   // This is the end of the SHT31 code and the beginning of the RPM counter code. Notice
   // that nothing special has to be done - just start writing the code for the next sensor.
   // You'll notice slight differences in code styles - for example, the code above
   // defines the config paths "in place", and the code below defines them with a variable,
   // then uses the variable as the function parameter. Either approach works.  

  const char* sk_path = "propulsion.main.revolutions";
  const char* config_path_calibrate = "/sensors/engine_rpm/calibrate";
  const char* config_path_skpath = "/sensors/engine_rpm/sk";
  const float multiplier = 1.0 / 97.0;
  const uint rpm_read_delay = 500;

#ifdef ESP8266
  uint8_t pin = D5;  
#elif defined(ESP32)
  uint8_t pin = 4;
#endif
  auto* dic = new DigitalInputCounter(pin, INPUT_PULLUP, RISING, rpm_read_delay);

  dic
      ->connect_to(new Frequency(multiplier, config_path_calibrate)) 
      ->connect_to(new SKOutputNumber(sk_path, config_path_skpath));

    // This is the end of the RPM counter code.

    // If you wanted to add a third, fourth, or more sensor, you would do that
    // here. An ESP9266 should easily handle four or five sensors, and an ESP32
    // should handle eight or ten, or more.
    
  sensesp_app->enable();
});
