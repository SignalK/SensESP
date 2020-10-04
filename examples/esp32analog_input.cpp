#include <Arduino.h>

#include "sensesp_app.h"
#include "sensesp_app_builder.h"

#include "signalk/signalk_output.h"

#include "sensors/esp32_analog_input.h"

ReactESP app([] () {

  // Some initialization boilerplate when in debug mode...
  #ifndef SERIAL_DEBUG_DISABLED
  Serial.begin(115200);

  // A small delay and one debugI() are required so that
  // the serial output displays everything
  delay(100);
  Debug.setSerialEnabled(true);
  #endif
  delay(100);
  debugI("Serial debug enabled");

  // Create the global SensESPApp() object. 
  // If you add the line ->set_wifi("your ssid", "your password") you can specify
  // the wifi parameters in the builder. If you do not do that, the SensESP 
  // device wifi configuration hotspot will appear and you can use a web 
  // browser pointed to 192.168.4.1 to configure the wifi parameters. 
  // You can use NONE, UPTIME, FREQUENCY, FREE_MEMORY, WIFI_SIGNAL or ALL 
  // instead of IP_ADDRESS in set_standard_sensors().

  // Create a builder object
  SensESPAppBuilder builder;

  sensesp_app = builder.set_hostname("test_unit")
                ->set_wifi("MysticBlue", "passwors1234")
                //->set_standard_sensors(IP_ADDRESS)
                ->set_standard_sensors(NONE)
                ->set_sk_server("192.168.1.141", 3000) 
                ->get_app();
  
// In the test setup the input voltage was 0.651 in which case all of the options below all 
// produced a similar ouput although the difference in resolution between the first and 
// last statement is clear - as expected.

  auto* pAnalogInput = new ESP32AnalogInput(34, ADC_WIDTH_BIT_12, ADC_ATTEN_DB_0, 1000);
//  auto* pAnalogInput = new ESP32AnalogInput(34, ADC_WIDTH_BIT_9, ADC_ATTEN_DB_0, 1000);
//  auto* pAnalogInput = new ESP32AnalogInput(34, ADC_WIDTH_BIT_12, ADC_ATTEN_DB_0, 1000);
//  auto* pAnalogInput = new ESP32AnalogInput(34, ADC_WIDTH_BIT_9, ADC_ATTEN_DB_11, 1000);
  
  pAnalogInput->connectTo(new SKOutputNumber("voltage")); 


  sensesp_app->enable();
  
});
