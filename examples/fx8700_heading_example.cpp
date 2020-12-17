#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <Wire.h>

#include <sstream>
#include <string>

#include "sensesp_app.h"
#include "sensors/orientation_9dof_input.h"
#include "sensors/sensor_nxp_fxos8700_fxas21002.h"
#include "signalk/signalk_output.h"

#define PIN_I2C_SDA (23)  // Adjust to your board. A value of -1
#define PIN_I2C_SCL (25)  // will use default Arduino pins.

#define ORIENTATION_SAMPLING_INTERVAL_MS (100)

// SensESP builds upon the ReactESP framework. Every ReactESP application
// defines an "app" object vs defining a "main()" method.
ReactESP app([]() {

// Some initialization boilerplate when in debug mode...
#ifndef SERIAL_DEBUG_DISABLED
  SetupSerialDebug(115200);
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
  const char* sk_path_heading = "orientation.heading";
  const char* sk_path_pitch = "orientation.pitch";
  const char* sk_path_roll = "orientation.roll";
  /* This example shows heading, pitch, and roll. If you want other parameters
      as well, uncomment the appropriate path(s) from the following.
  */
  // const char* sk_path_accel_x     = "orientation.accel_x";
  // const char* sk_path_accel_y     = "orientation.accel_y";
  // const char* sk_path_accel_z     = "orientation.accel_z";
  // const char* sk_path_turn_rate   = "orientation.turn_rate";
  // const char* sk_path_roll_rate   = "orientation.roll_rate";
  // const char* sk_path_pitch_rate  = "orientation.pitch_rate";

  // The "Configuration path" is combined with "/config" to formulate a URL
  // used by the RESTful API for retrieving or setting configuration data.
  // It is ALSO used to specify a path to the SPIFFS file system
  // where configuration data is saved on the MCU board. It should
  // ALWAYS start with a forward slash if specified. If left blank,
  // that indicates this sensor or transform does not have any
  // configuration to save, or that you're not interested in doing
  // run-time configuration.
  // These two are necessary until a method is created to synthesize them.
  const char* config_path_orientation_skpath = "/sensors/orientation/sk";
  const char* config_path_heading = "/sensors/orientation/heading";
  const char* config_path_pitch = "/sensors/orientation/pitch";
  const char* config_path_roll = "/sensors/orientation/roll";
  /* This example shows heading, pitch, and roll. If you want other parameters
      as well, uncomment the appropriate path(s) from the following.
  */
  // const char* config_path_accel_x = "/sensors/orientation/accel_x"; 
  // const char* config_path_accel_y = "/sensors/orientation/accel_y"; 
  // const char* config_path_accel_z = "/sensors/orientation/accel_z"; 
  // const char* config_path_turn_rate = "/sensors/orientation/turn_rate"; 
  // const char* config_path_roll_rate = "/sensors/orientation/roll_rate";
  // const char* config_path_pitch_rate = "/sensors/orientation/pitch_rate";
  /* above arrangement of paths yields this sensor web interface structure:
     sensors->orientation->heading
                         ->pitch
                         ->roll
                         ->sk   (and this one, which allows setting the SK name
     of sensor, only lists the last connect_to path, "orientation.roll" in this
     case)
  */

  // Magnetic Heading data source, using 9 Degrees-of-freedom combination sensor
  auto* NXP9DOF = new Orientation9DOF(PIN_I2C_SDA, PIN_I2C_SCL, config_path_heading);

  /*  Uncomment the following line during calibration - this will stream raw
     readings to serial port, where they can be intercepted by the Motion Sensor
     Calibration Tool. See Adafruit tutorial
      https://learn.adafruit.com/adafruit-sensorlab-magnetometer-calibration/magnetic-calibration-with-motioncal
      and Paul Stoffregen's app https://github.com/PaulStoffregen/MotionCal
      After calibration values have been written to EEPROM/Flash, re-comment
     following line to restore normal program flow.
  */
  //  NXP9DOF->stream_raw_values();  //continuous raw data collection. This call
  //  does not return.

  // Start periodic readings from orientation sensor. Note that the physical
  //  sensor is read at whatever rate is specified for the heading
  //  parameter; all other parameters are retrieved from variables that
  //  are updated only at the time of the sensor read. So, ensure that
  //  the sampling rate specified for the heading parameter is the same
  //  or faster than any of the other parameters (otherwise those other
  //  parameters will not update at the rate you expect).
  auto* sensor_heading =
      new Read9DOF(NXP9DOF, Read9DOF::compass_hdg, ORIENTATION_SAMPLING_INTERVAL_MS,
                   config_path_heading);
  sensor_heading->connect_to(
      new SKOutputNumber(sk_path_heading, config_path_orientation_skpath));

  auto* sensor_pitch = new Read9DOF(
      NXP9DOF, Read9DOF::pitch, ORIENTATION_SAMPLING_INTERVAL_MS * 5, config_path_pitch);
  sensor_pitch->connect_to(
      new SKOutputNumber(sk_path_pitch, config_path_orientation_skpath));

  auto* sensor_roll = new Read9DOF(
      NXP9DOF, Read9DOF::roll, ORIENTATION_SAMPLING_INTERVAL_MS * 5, config_path_roll);
  sensor_roll->connect_to(
      new SKOutputNumber(sk_path_roll, config_path_orientation_skpath));

  /* This example shows heading, pitch, and roll. If you want other parameters
      as well, uncomment the appropriate connections from the following.

 auto* sensor_turn_rate =
      new Read9DOF(NXP9DOF, Read9DOF::rate_of_turn, ORIENTATION_SAMPLING_INTERVAL_MS*5,
                   config_path_turn_rate);
  sensor_turn_rate->connect_to(
      new SKOutputNumber(sk_path_turn_rate, config_path_orientation_skpath));

 auto* sensor_roll_rate =
      new Read9DOF(NXP9DOF, Read9DOF::rate_of_roll, ORIENTATION_SAMPLING_INTERVAL_MS*5,
                   config_path_roll_rate);
  sensor_roll_rate->connect_to(
      new SKOutputNumber(sk_path_roll_rate, config_path_orientation_skpath));

 auto* sensor_pitch_rate =
      new Read9DOF(NXP9DOF, Read9DOF::rate_of_pitch, ORIENTATION_SAMPLING_INTERVAL_MS*5,
                   config_path_pitch_rate);
  sensor_pitch_rate->connect_to(
      new SKOutputNumber(sk_path_pitch_rate, config_path_orientation_skpath));

 auto* sensor_accel_x =
      new Read9DOF(NXP9DOF, Read9DOF::acceleration_x, ORIENTATION_SAMPLING_INTERVAL_MS*5,
                   config_path_accel_x);
  sensor_accel_x->connect_to(
      new SKOutputNumber(sk_path_accel_x, config_path_orientation_skpath));

 auto* sensor_accel_y =
      new Read9DOF(NXP9DOF, Read9DOF::acceleration_y, ORIENTATION_SAMPLING_INTERVAL_MS*5,
                   config_path_accel_y);
  sensor_accel_y->connect_to(
      new SKOutputNumber(sk_path_accel_y, config_path_orientation_skpath));

 auto* sensor_accel_z =
      new Read9DOF(NXP9DOF, Read9DOF::acceleration_z, ORIENTATION_SAMPLING_INTERVAL_MS*5,
                   config_path_accel_z);
  sensor_accel_z->connect_to(
      new SKOutputNumber(sk_path_accel_z, config_path_orientation_skpath));
  */
  /*  Relationship of the Axes and the terminology:
  X,Y,Z are orthogonal, in a Right-handed coordinate system:
  think of the X axis pointing West, Y pointing South, and Z pointing up.
  On the Adafruit FXOS8700/FXAS21002 sensor PCB, the axes are printed
  on the top-side silkscreen.
  Acceleration is measured in the direction of the corresponding axis.

  If the sensor is mounted with the X-axis pointing to the bow of the boat
  and the Y-axis pointing to Port, then Z points up and the following applies:
  Heading is rotation about the Z-axis. It increases with rotation to starboard.
  Pitch is rotation about the Y-axis. Positive is when the bow points up.
  Roll is rotation about the X-axis. Positive is rolling to starboard.
  Turn-rate is rotation about the Z-axis. Positive is increasing bearing.
  Roll-rate is rotation about the X-axis. Positive is rolling to starboard.
  Pitch-rate is rotation about the Y-axis. Positive is bow rising.

  If the sensor is mounted differently, or you prefer an alternate nomenclature,
  the get___() methods in sensor_NXP_FXOS8700_FXAS21002.cpp can be adjusted.
*/

  // Start the SensESP application running
  sensesp_app->enable();
});
