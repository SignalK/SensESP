#include <Arduino.h>
#include <Wire.h>

#include "SPIFFS.h"

#include <sstream>
#include <string>

#include <Adafruit_Sensor.h>

#include "sensesp_app.h"
#include "signalk/signalk_output.h"
#include "sensors/orientation_9DOF_input.h"
#include "sensors/sensor_NXP_FXOS8700_FXAS21002.h"

#define PIN_I2C_SDA   (23)  //Adjust to your board. A value of -1
#define PIN_I2C_SCL   (25)  // will use default Arduino pins.

#define ORIENTATION_SAMPLING_INTERVAL_MS (100)

// SensESP builds upon the ReactESP framework. Every ReactESP application
// defines an "app" object vs defining a "main()" method.
ReactESP app([]() {

// Some initialization boilerplate when in debug mode...
#ifndef SERIAL_DEBUG_DISABLED
  Serial.begin(115200);
  // A small delay and one debugI() are required so that
  // the serial output displays everything
  delay(100);
  Debug.setSerialEnabled(true);
  delay(100);
  debugI("Serial debug enabled");
#endif

  // Create the global SensESPApp() object.
  sensesp_app = new SensESPApp();

  // The "SignalK path" identifies this sensor to the SignalK server. Leaving
  // this blank would indicate this particular sensor (or transform) does not
  // broadcast SignalK data.
  // If you have multiple sensors connected to your microcontoller (ESP), each
  // one of them will (probably) have its own SignalK path variable. For
  // example, if you have two propulsion engines, and you want the RPM of each
  // of them to go to SignalK, you might have sk_path_portEngine =
  // "propulsion.port.revolutions" and sk_path_starboardEngine =
  // "propulsion.starboard.revolutions"
  const char* sk_path_hdg = "compass.magnetometer";

  // The "Configuration path" is combined with "/config" to formulate a URL
  // used by the RESTful API for retrieving or setting configuration data.
  // It is ALSO used to specify a path to the SPIFFS file system
  // where configuration data is saved on the MCU board. It should
  // ALWAYS start with a forward slash if specified. If left blank,
  // that indicates this sensor or transform does not have any
  // configuration to save, or that you're not interested in doing
  // run-time configuration.
  // These two are necessary until a method is created to synthesize them.
  // Everything after "/sensors" in each of these ("/engine_rpm/calibrate" and
  // "/engine_rpm/sk") is simply a label to display what you're configuring in
  // the Configuration UI.
  const char* config_path_hdg_skpath = "/sensors/orientation/sk";
  const char* config_path_hdg_compass = "/sensors/orientation/compass";

  // Magnetic Heading data source, using 9 Degrees-of-freedom combination sensor
  Orientation9DOF NXP9DOF =
      Orientation9DOF(PIN_I2C_SDA, PIN_I2C_SCL, config_path_hdg_compass);

  /*  Uncomment the following line during calibration - this will stream raw
     readings to serial port, where they can be intercepted by the Motion Sensor
     Calibration Tool. See Adafruit tutorial
      https://learn.adafruit.com/adafruit-sensorlab-magnetometer-calibration/magnetic-calibration-with-motioncal
      and Paul Stoffregen's app https://github.com/PaulStoffregen/MotionCal
      After calibration values have been written to EEPROM/Flash, re-comment
     following line to restore normal program flow.
  */
  //  NXP9DOF.streamRawValues();  //continuous raw data collection. This call does not return.

  // start periodic readings from magnetometer
  auto* pSensor_compass =
      new Read9DOF(&NXP9DOF, compass_hdg, ORIENTATION_SAMPLING_INTERVAL_MS,
                   config_path_hdg_compass);
  pSensor_compass->connectTo(
      new SKOutputNumber(sk_path_hdg, config_path_hdg_skpath));

  // Start the SensESP application running
  sensesp_app->enable();
});